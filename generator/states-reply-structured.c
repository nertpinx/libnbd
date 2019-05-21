/* nbd client library in userspace: state machine
 * Copyright (C) 2013-2019 Red Hat Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/* State machine for parsing structured replies from the server. */

/* STATE MACHINE */ {
 REPLY.STRUCTURED_REPLY.START:
  /* We've only read the simple_reply.  The structured_reply is longer,
   * so read the remaining part.
   */
  if (!conn->structured_replies) {
    set_error (0, "server sent unexpected structured reply");
    SET_NEXT_STATE(%.DEAD);
    return 0;
  }
  conn->rbuf = &conn->sbuf;
  conn->rbuf += sizeof conn->sbuf.simple_reply;
  conn->rlen = sizeof conn->sbuf.sr.structured_reply;
  conn->rlen -= sizeof conn->sbuf.simple_reply;
  SET_NEXT_STATE (%RECV_REMAINING);
  return 0;

 REPLY.STRUCTURED_REPLY.RECV_REMAINING:
  switch (recv_into_rbuf (conn)) {
  case -1: SET_NEXT_STATE (%.DEAD); return -1;
  case 0:  SET_NEXT_STATE (%CHECK);
  }
  return 0;

 REPLY.STRUCTURED_REPLY.CHECK:
  struct command_in_flight *cmd;
  uint16_t flags, type;
  uint64_t handle;
  uint32_t length;

  flags = be16toh (conn->sbuf.sr.structured_reply.flags);
  type = be16toh (conn->sbuf.sr.structured_reply.type);
  handle = be64toh (conn->sbuf.sr.structured_reply.handle);
  length = be32toh (conn->sbuf.sr.structured_reply.length);

  /* Find the command amongst the commands in flight. */
  for (cmd = conn->cmds_in_flight; cmd != NULL; cmd = cmd->next) {
    if (cmd->handle == handle)
      break;
  }
  if (cmd == NULL) {
    /* Unlike for simple replies, this is difficult to recover from.  We
     * would need an extra state to read and ignore length bytes. XXX
     */
    SET_NEXT_STATE (%.DEAD);
    set_error (0, "no matching handle found for server reply, "
               "this is probably a bug in the server");
    return -1;
  }

  if (NBD_REPLY_TYPE_IS_ERR (type)) {
    if (length < sizeof conn->sbuf.sr.payload.error) {
      SET_NEXT_STATE (%.DEAD);
      set_error (0, "too short length in structured reply error");
      return -1;
    }
    conn->rbuf = &conn->sbuf.sr.payload.error;
    conn->rlen = sizeof conn->sbuf.sr.payload.error;
    SET_NEXT_STATE (%RECV_ERROR);
    return 0;
  }
  else if (type == NBD_REPLY_TYPE_NONE) {
    if (length != 0) {
      SET_NEXT_STATE (%.DEAD);
      set_error (0, "invalid length in NBD_REPLY_TYPE_NONE");
      return -1;
    }
    if (!(flags & NBD_REPLY_FLAG_DONE)) {
      SET_NEXT_STATE (%.DEAD);
      set_error (0, "NBD_REPLY_FLAG_DONE must be set in NBD_REPLY_TYPE_NONE");
      return -1;
    }
    SET_NEXT_STATE (%^FINISH_COMMAND);
    return 0;
  }
  else if (type == NBD_REPLY_TYPE_OFFSET_DATA) {
    if (length < sizeof conn->sbuf.sr.payload.offset_data) {
      SET_NEXT_STATE (%.DEAD);
      set_error (0, "too short length in NBD_REPLY_TYPE_OFFSET_DATA");
      return -1;
    }
    conn->rbuf = &conn->sbuf.sr.payload.offset_data;
    conn->rlen = sizeof conn->sbuf.sr.payload.offset_data;
    SET_NEXT_STATE (%RECV_OFFSET_DATA);
    return 0;
  }
  else if (type == NBD_REPLY_TYPE_OFFSET_HOLE) {
    if (length != 12) {
      SET_NEXT_STATE (%.DEAD);
      set_error (0, "invalid length in NBD_REPLY_TYPE_NONE");
      return -1;
    }
    conn->rbuf = &conn->sbuf.sr.payload.offset_hole;
    conn->rlen = sizeof conn->sbuf.sr.payload.offset_hole;
    SET_NEXT_STATE (%RECV_OFFSET_HOLE);
    return 0;
  }
  else if (type == NBD_REPLY_TYPE_BLOCK_STATUS) {
    /* XXX We should be able to skip the bad reply in these two cases. */
    if (length < 12 || ((length-4) & 7) != 0) {
      SET_NEXT_STATE (%.DEAD);
      set_error (0, "invalid length in NBD_REPLY_TYPE_BLOCK_STATUS");
      return -1;
    }
    if (cmd->extent_fn == NULL) {
      SET_NEXT_STATE (%.DEAD);
      set_error (0, "not expecting NBD_REPLY_TYPE_BLOCK_STATUS here");
      return -1;
    }
    /* We read the context ID followed by all the entries into a
     * single array and deal with it at the end.
     */
    free (conn->bs_entries);
    conn->bs_entries = malloc (length);
    if (conn->bs_entries == NULL) {
      SET_NEXT_STATE (%.DEAD);
      set_error (errno, "malloc");
      return -1;
    }
    conn->rbuf = conn->bs_entries;
    conn->rlen = length;
    SET_NEXT_STATE (%RECV_BS_ENTRIES);
    return 0;
  }
  else {
    SET_NEXT_STATE (%.DEAD);
    set_error (0, "unknown structured reply type (%" PRIu16 ")", type);
    return -1;
  }

 REPLY.STRUCTURED_REPLY.RECV_ERROR:
  switch (recv_into_rbuf (conn)) {
  case -1: SET_NEXT_STATE (%.DEAD); return -1;
  case 0:
    /* We skip the human readable error for now. XXX */
    conn->rbuf = NULL;
    conn->rlen = be16toh (conn->sbuf.sr.payload.error.len);
    SET_NEXT_STATE (%RECV_ERROR_MESSAGE);
  }
  return 0;

 REPLY.STRUCTURED_REPLY.RECV_ERROR_MESSAGE:
  struct command_in_flight *cmd;
  uint16_t flags;
  uint64_t handle;
  uint32_t error;

  switch (recv_into_rbuf (conn)) {
  case -1: SET_NEXT_STATE (%.DEAD); return -1;
  case 0:
    flags = be16toh (conn->sbuf.sr.structured_reply.flags);
    handle = be64toh (conn->sbuf.sr.structured_reply.handle);
    error = be32toh (conn->sbuf.sr.payload.error.error);

    /* Find the command amongst the commands in flight. */
    for (cmd = conn->cmds_in_flight; cmd != NULL; cmd = cmd->next) {
      if (cmd->handle == handle)
        break;
    }
    assert (cmd); /* guaranteed by CHECK */

    cmd->error = error;

    if (flags & NBD_REPLY_FLAG_DONE)
      SET_NEXT_STATE (%^FINISH_COMMAND);
    else
      SET_NEXT_STATE (%.READY);
  }
  return 0;

 REPLY.STRUCTURED_REPLY.RECV_OFFSET_DATA:
  struct command_in_flight *cmd;
  uint64_t handle;
  uint64_t offset;
  uint32_t length;

  switch (recv_into_rbuf (conn)) {
  case -1: SET_NEXT_STATE (%.DEAD); return -1;
  case 0:
    handle = be64toh (conn->sbuf.sr.structured_reply.handle);
    length = be32toh (conn->sbuf.sr.structured_reply.length);
    offset = be64toh (conn->sbuf.sr.payload.offset_data.offset);

    /* Find the command amongst the commands in flight. */
    for (cmd = conn->cmds_in_flight; cmd != NULL; cmd = cmd->next) {
      if (cmd->handle == handle)
        break;
    }
    assert (cmd); /* guaranteed by CHECK */

    if (cmd->type != NBD_CMD_READ) {
      SET_NEXT_STATE (%.DEAD);
      set_error (0, "invalid command for receiving offset-data chunk, "
                 "cmd->type=%" PRIu16 ", "
                 "this is likely to be a bug in the server",
                 cmd->type);
      return -1;
    }
    assert (cmd->data);

    /* Length of the data following. */
    length -= 8;

    /* Is the data within bounds? */
    if (offset < cmd->offset) {
      SET_NEXT_STATE (%.DEAD);
      set_error (0, "offset of reply is out of bounds, "
                 "offset=%" PRIu64 ", cmd->offset=%" PRIu64 ", "
                 "this is likely to be a bug in the server",
                 offset, cmd->offset);
      return -1;
    }
    /* Now this is the byte offset in the read buffer. */
    offset -= cmd->offset;

    if (offset + length > cmd->count) {
      SET_NEXT_STATE (%.DEAD);
      set_error (0, "offset/length of reply is out of bounds, "
                 "offset=%" PRIu64 ", length=%" PRIu32 ", "
                 "cmd->count=%" PRIu32 ", "
                 "this is likely to be a bug in the server",
                 offset, length, cmd->count);
      return -1;
    }

    /* Set up to receive the data directly to the user buffer. */
    conn->rbuf = cmd->data + offset;
    conn->rlen = length;
    SET_NEXT_STATE (%RECV_OFFSET_DATA_DATA);
  }
  return 0;

 REPLY.STRUCTURED_REPLY.RECV_OFFSET_DATA_DATA:
  uint16_t flags;

  switch (recv_into_rbuf (conn)) {
  case -1: SET_NEXT_STATE (%.DEAD); return -1;
  case 0:
    flags = be16toh (conn->sbuf.sr.structured_reply.flags);

    if (flags & NBD_REPLY_FLAG_DONE)
      SET_NEXT_STATE (%^FINISH_COMMAND);
    else
      SET_NEXT_STATE (%.READY);
  }
  return 0;

 REPLY.STRUCTURED_REPLY.RECV_OFFSET_HOLE:
  struct command_in_flight *cmd;
  uint64_t handle;
  uint16_t flags;
  uint64_t offset;
  uint32_t length;

  switch (recv_into_rbuf (conn)) {
  case -1: SET_NEXT_STATE (%.DEAD); return -1;
  case 0:
    handle = be64toh (conn->sbuf.sr.structured_reply.handle);
    flags = be16toh (conn->sbuf.sr.structured_reply.flags);
    offset = be64toh (conn->sbuf.sr.payload.offset_hole.offset);
    length = be32toh (conn->sbuf.sr.payload.offset_hole.length);

    /* Find the command amongst the commands in flight. */
    for (cmd = conn->cmds_in_flight; cmd != NULL; cmd = cmd->next) {
      if (cmd->handle == handle)
        break;
    }
    assert (cmd); /* guaranteed by CHECK */

    if (cmd->type != NBD_CMD_READ) {
      SET_NEXT_STATE (%.DEAD);
      set_error (0, "invalid command for receiving offset-hole chunk, "
                 "cmd->type=%" PRIu16 ", "
                 "this is likely to be a bug in the server",
                 cmd->type);
      return -1;
    }
    assert (cmd->data);

    /* Is the data within bounds? */
    if (offset < cmd->offset) {
      SET_NEXT_STATE (%.DEAD);
      set_error (0, "offset of reply is out of bounds, "
                 "offset=%" PRIu64 ", cmd->offset=%" PRIu64 ", "
                 "this is likely to be a bug in the server",
                 offset, cmd->offset);
      return -1;
    }
    /* Now this is the byte offset in the read buffer. */
    offset -= cmd->offset;

    if (offset + length > cmd->count) {
      SET_NEXT_STATE (%.DEAD);
      set_error (0, "offset/length of reply is out of bounds, "
                 "offset=%" PRIu64 ", length=%" PRIu32 ", "
                 "cmd->count=%" PRIu32 ", "
                 "this is likely to be a bug in the server",
                 offset, length, cmd->count);
      return -1;
    }

    memset (cmd->data + offset, 0, length);

    if (flags & NBD_REPLY_FLAG_DONE)
      SET_NEXT_STATE (%^FINISH_COMMAND);
    else
      SET_NEXT_STATE (%.READY);
  }
  return 0;

 REPLY.STRUCTURED_REPLY.RECV_BS_ENTRIES:
  struct command_in_flight *cmd;
  uint64_t handle;
  uint16_t flags;
  uint32_t length;
  size_t i;
  uint32_t context_id;
  struct meta_context *meta_context;

  switch (recv_into_rbuf (conn)) {
  case -1: SET_NEXT_STATE (%.DEAD); return -1;
  case 0:
    handle = be64toh (conn->sbuf.sr.structured_reply.handle);
    flags = be16toh (conn->sbuf.sr.structured_reply.flags);
    length = be32toh (conn->sbuf.sr.structured_reply.length);

    /* Find the command amongst the commands in flight. */
    for (cmd = conn->cmds_in_flight; cmd != NULL; cmd = cmd->next) {
      if (cmd->handle == handle)
        break;
    }
    /* guaranteed by CHECK */
    assert (cmd);
    assert (cmd->extent_fn);
    assert (conn->bs_entries);
    assert (length >= 12);

    /* Need to byte-swap the entries returned, but apart from that we
     * don't validate them.
     */
    for (i = 0; i < length/4; ++i)
      conn->bs_entries[i] = be32toh (conn->bs_entries[i]);

    /* Look up the context ID. */
    context_id = conn->bs_entries[0];
    for (meta_context = conn->meta_contexts;
         meta_context;
         meta_context = meta_context->next)
      if (context_id == meta_context->context_id)
        break;

    if (meta_context)
      /* Call the caller's extent function. */
      cmd->extent_fn (cmd->data, meta_context->name, cmd->offset,
                      &conn->bs_entries[1], (length-4) / 4);
    else
      /* Emit a debug message, but ignore it. */
      debug (h, "server sent unexpected meta context ID %" PRIu32,
             context_id);

    if (flags & NBD_REPLY_FLAG_DONE)
      SET_NEXT_STATE (%^FINISH_COMMAND);
    else
      SET_NEXT_STATE (%.READY);
  }
  return 0;

} /* END STATE MACHINE */