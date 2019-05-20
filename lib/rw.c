/* NBD client library in userspace
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

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>

#include "internal.h"

/* For synchronous functions, this picks which connection to use.  It
 * has simple round-robin behaviour, but ignores connections which are
 * dead or not ready.  It will return an error if there are no
 * suitable connections.
 */
static struct nbd_connection *
pick_connection (struct nbd_handle *h)
{
  size_t i, j;
  struct nbd_connection *conn = NULL;
  int error = ENOTCONN;

  i = h->unique % h->multi_conn;
  for (j = 0; j < h->multi_conn; ++j) {
    if (nbd_unlocked_aio_is_ready (h->conns[i])) {
      conn = h->conns[i];
      break;
    }
    /* At least one connection is busy, not dead and not sitting in
     * the initial CREATED state.
     */
    if (h->conns[i]->state != STATE_START &&
        !nbd_unlocked_aio_is_dead (h->conns[i]))
      error = EBUSY;

    ++i;
    if (i >= h->multi_conn)
      i = 0;
  }

  if (conn == NULL) {
    set_error (error, "no connection(s) are ready to issue commands");
    return NULL;
  }

  return conn;
}

/* Issue a read command on any connection and wait for the reply. */
int
nbd_unlocked_pread (struct nbd_handle *h, void *buf,
                    size_t count, uint64_t offset)
{
  struct nbd_connection *conn;
  int64_t ch;
  int r;

  conn = pick_connection (h);
  if (conn == NULL)
    return -1;

  ch = nbd_unlocked_aio_pread (conn, buf, count, offset);
  if (ch == -1)
    return -1;

  while ((r = nbd_unlocked_aio_command_completed (conn, ch)) == 0) {
    if (nbd_unlocked_poll (h, -1) == -1)
      return -1;
  }

  return r == -1 ? -1 : 0;
}

/* Issue a write command on any connection and wait for the reply. */
int
nbd_unlocked_pwrite (struct nbd_handle *h, const void *buf,
                     size_t count, uint64_t offset, uint32_t flags)
{
  struct nbd_connection *conn;
  int64_t ch;
  int r;

  conn = pick_connection (h);
  if (conn == NULL)
    return -1;

  ch = nbd_unlocked_aio_pwrite (conn, buf, count, offset, flags);
  if (ch == -1)
    return -1;

  while ((r = nbd_unlocked_aio_command_completed (conn, ch)) == 0) {
    if (nbd_unlocked_poll (h, -1) == -1)
      return -1;
  }

  return r == -1 ? -1 : 0;
}

/* Issue a flush command on any connection and wait for the reply. */
int
nbd_unlocked_flush (struct nbd_handle *h)
{
  struct nbd_connection *conn;
  int64_t ch;
  int r;

  conn = pick_connection (h);
  if (conn == NULL)
    return -1;

  ch = nbd_unlocked_aio_flush (conn);
  if (ch == -1)
    return -1;

  while ((r = nbd_unlocked_aio_command_completed (conn, ch)) == 0) {
    if (nbd_unlocked_poll (h, -1) == -1)
      return -1;
  }

  return r == -1 ? -1 : 0;
}

/* Issue a trim command on any connection and wait for the reply. */
int
nbd_unlocked_trim (struct nbd_handle *h,
                   uint64_t count, uint64_t offset, uint32_t flags)
{
  struct nbd_connection *conn;
  int64_t ch;
  int r;

  conn = pick_connection (h);
  if (conn == NULL)
    return -1;

  ch = nbd_unlocked_aio_trim (conn, count, offset, flags);
  if (ch == -1)
    return -1;

  while ((r = nbd_unlocked_aio_command_completed (conn, ch)) == 0) {
    if (nbd_unlocked_poll (h, -1) == -1)
      return -1;
  }

  return r == -1 ? -1 : 0;
}

/* Issue a cache command on any connection and wait for the reply. */
int
nbd_unlocked_cache (struct nbd_handle *h,
                    uint64_t count, uint64_t offset)
{
  struct nbd_connection *conn;
  int64_t ch;
  int r;

  conn = pick_connection (h);
  if (conn == NULL)
    return -1;

  ch = nbd_unlocked_aio_cache (conn, count, offset);
  if (ch == -1)
    return -1;

  while ((r = nbd_unlocked_aio_command_completed (conn, ch)) == 0) {
    if (nbd_unlocked_poll (h, -1) == -1)
      return -1;
  }

  return r == -1 ? -1 : 0;
}

/* Issue a zero command on any connection and wait for the reply. */
int
nbd_unlocked_zero (struct nbd_handle *h,
                   uint64_t count, uint64_t offset, uint32_t flags)
{
  struct nbd_connection *conn;
  int64_t ch;
  int r;

  conn = pick_connection (h);
  if (conn == NULL)
    return -1;

  ch = nbd_unlocked_aio_zero (conn, count, offset, flags);
  if (ch == -1)
    return -1;

  while ((r = nbd_unlocked_aio_command_completed (conn, ch)) == 0) {
    if (nbd_unlocked_poll (h, -1) == -1)
      return -1;
  }

  return r == -1 ? -1 : 0;
}

/* Issue a block status command on any connection and wait for the reply. */
int
nbd_unlocked_block_status (struct nbd_handle *h,
                           uint64_t count, uint64_t offset, uint32_t flags,
                           int64_t id,
                           extent_fn extent)
{
  struct nbd_connection *conn;
  int64_t ch;
  int r;

  conn = pick_connection (h);
  if (conn == NULL)
    return -1;

  ch = nbd_unlocked_aio_block_status (conn, count, offset, flags, id, extent);
  if (ch == -1)
    return -1;

  while ((r = nbd_unlocked_aio_command_completed (conn, ch)) == 0) {
    if (nbd_unlocked_poll (h, -1) == -1)
      return -1;
  }

  return r == -1 ? -1 : 0;
}

static struct command_in_flight *
command_common (struct nbd_connection *conn,
                uint16_t flags, uint16_t type,
                uint64_t offset, uint64_t count, void *data)
{
  struct command_in_flight *cmd;

  switch (type) {
    /* Commands which send or receive data are limited to MAX_REQUEST_SIZE. */
  case NBD_CMD_READ:
  case NBD_CMD_WRITE:
    if (count > MAX_REQUEST_SIZE) {
      set_error (ERANGE, "request too large: maximum request size is %d",
                 MAX_REQUEST_SIZE);
      return NULL;
    }
    break;

    /* Other commands are currently limited by the 32 bit field in the
     * command structure on the wire, but in future we hope to support
     * 64 bit values here with a change to the NBD protocol which is
     * being discussed upstream.
     */
  default:
    if (count > UINT32_MAX) {
      set_error (ERANGE, "request too large: maximum request size is %" PRIu32,
                 UINT32_MAX);
      return NULL;
    }
    break;
  }

  cmd = calloc (1, sizeof *cmd);
  if (cmd == NULL) {
    set_error (errno, "calloc");
    return NULL;
  }
  cmd->flags = flags;
  cmd->type = type;
  cmd->handle = conn->h->unique++;
  cmd->offset = offset;
  cmd->count = count;
  cmd->data = data;

  /* If structured replies were negotiated then we trust the server to
   * send back sufficient data to cover the whole buffer.  It's tricky
   * to check this, so an easier thing is simply to zero the buffer
   * ahead of time which avoids any security problems.  I measured the
   * overhead of this and for non-TLS there is no measurable overhead
   * in the highly intensive loopback case.  For TLS we get a
   * performance gain, go figure.
   */
  if (conn->structured_replies && cmd->data && type == NBD_CMD_READ)
    memset (cmd->data, 0, cmd->count);

  cmd->next = conn->cmds_to_issue;
  conn->cmds_to_issue = cmd;

  return cmd;
}

int64_t
nbd_unlocked_aio_pread (struct nbd_connection *conn, void *buf,
                        size_t count, uint64_t offset)
{
  struct command_in_flight *cmd;

  cmd = command_common (conn, 0, NBD_CMD_READ, offset, count, buf);
  if (!cmd)
    return -1;
  if (nbd_internal_run (conn->h, conn, cmd_issue) == -1)
    return -1;

  return cmd->handle;
}

int64_t
nbd_unlocked_aio_pwrite (struct nbd_connection *conn, const void *buf,
                         size_t count, uint64_t offset,
                         uint32_t flags)
{
  struct command_in_flight *cmd;

  if (nbd_unlocked_read_only (conn->h) == 1) {
    set_error (EINVAL, "server does not support write operations");
    return -1;
  }

  if ((flags & ~LIBNBD_CMD_FLAG_FUA) != 0) {
    set_error (EINVAL, "invalid flag: %" PRIu32, flags);
    return -1;
  }

  if ((flags & LIBNBD_CMD_FLAG_FUA) != 0 &&
      nbd_unlocked_can_fua (conn->h) != 1) {
    set_error (EINVAL, "server does not support the FUA flag");
    return -1;
  }

  cmd = command_common (conn, flags, NBD_CMD_WRITE, offset, count,
                        (void *) buf);
  if (!cmd)
    return -1;
  if (nbd_internal_run (conn->h, conn, cmd_issue) == -1)
    return -1;

  return cmd->handle;
}

int64_t
nbd_unlocked_aio_flush (struct nbd_connection *conn)
{
  struct command_in_flight *cmd;

  if (nbd_unlocked_can_flush (conn->h) != 1) {
    set_error (EINVAL, "server does not support flush operations");
    return -1;
  }

  cmd = command_common (conn, 0, NBD_CMD_FLUSH, 0, 0, NULL);
  if (!cmd)
    return -1;
  if (nbd_internal_run (conn->h, conn, cmd_issue) == -1)
    return -1;

  return cmd->handle;
}

int64_t
nbd_unlocked_aio_trim (struct nbd_connection *conn,
                       uint64_t count, uint64_t offset,
                       uint32_t flags)
{
  struct command_in_flight *cmd;

  if (nbd_unlocked_read_only (conn->h) == 1) {
    set_error (EINVAL, "server does not support write operations");
    return -1;
  }

  if ((flags & ~LIBNBD_CMD_FLAG_FUA) != 0) {
    set_error (EINVAL, "invalid flag: %" PRIu32, flags);
    return -1;
  }

  if ((flags & LIBNBD_CMD_FLAG_FUA) != 0 &&
      nbd_unlocked_can_fua (conn->h) != 1) {
    set_error (EINVAL, "server does not support the FUA flag");
    return -1;
  }

  if (count == 0) {             /* NBD protocol forbids this. */
    set_error (EINVAL, "count cannot be 0");
    return -1;
  }

  cmd = command_common (conn, flags, NBD_CMD_TRIM, offset, count, NULL);
  if (!cmd)
    return -1;
  if (nbd_internal_run (conn->h, conn, cmd_issue) == -1)
    return -1;

  return cmd->handle;
}

int64_t
nbd_unlocked_aio_cache (struct nbd_connection *conn,
                        uint64_t count, uint64_t offset)
{
  struct command_in_flight *cmd;

  /* Actually according to the NBD protocol document, servers do exist
   * that support NBD_CMD_CACHE but don't advertize the
   * NBD_FLAG_SEND_CACHE bit, but we ignore those.
   */
  if (nbd_unlocked_can_cache (conn->h) != 1) {
    set_error (EINVAL, "server does not support cache operations");
    return -1;
  }

  cmd = command_common (conn, 0, NBD_CMD_CACHE, offset, count, NULL);
  if (!cmd)
    return -1;
  if (nbd_internal_run (conn->h, conn, cmd_issue) == -1)
    return -1;

  return cmd->handle;
}

int64_t
nbd_unlocked_aio_zero (struct nbd_connection *conn,
                       uint64_t count, uint64_t offset,
                       uint32_t flags)
{
  struct command_in_flight *cmd;

  if (nbd_unlocked_read_only (conn->h) == 1) {
    set_error (EINVAL, "server does not support write operations");
    return -1;
  }

  if ((flags & ~LIBNBD_CMD_FLAG_FUA) != 0) {
    set_error (EINVAL, "invalid flag: %" PRIu32, flags);
    return -1;
  }

  if ((flags & LIBNBD_CMD_FLAG_FUA) != 0 &&
      nbd_unlocked_can_fua (conn->h) != 1) {
    set_error (EINVAL, "server does not support the FUA flag");
    return -1;
  }

  if (count == 0) {             /* NBD protocol forbids this. */
    set_error (EINVAL, "count cannot be 0");
    return -1;
  }

  cmd = command_common (conn, flags, NBD_CMD_WRITE_ZEROES, offset, count, NULL);
  if (!cmd)
    return -1;
  if (nbd_internal_run (conn->h, conn, cmd_issue) == -1)
    return -1;

  return cmd->handle;
}

int64_t
nbd_unlocked_aio_block_status (struct nbd_connection *conn,
                               uint64_t count, uint64_t offset,
                               uint32_t flags,
                               int64_t id,
                               extent_fn extent)
{
  struct command_in_flight *cmd;

  if (!conn->structured_replies) {
    set_error (ENOTSUP, "server does not support structured replies");
    return -1;
  }

  if (conn->meta_contexts == NULL) {
    set_error (ENOTSUP, "did not negotiate any metadata contexts, "
               "either you did not call nbd_request_meta_context before "
               "connecting or the server does not support it");
    return -1;
  }

  if ((flags & ~LIBNBD_CMD_FLAG_REQ_ONE) != 0) {
    set_error (EINVAL, "invalid flag: %" PRIu32, flags);
    return -1;
  }

  if (count == 0) {             /* NBD protocol forbids this. */
    set_error (EINVAL, "count cannot be 0");
    return -1;
  }

  cmd = command_common (conn, flags, NBD_CMD_BLOCK_STATUS, offset, count, NULL);
  if (!cmd)
    return -1;

  cmd->extent_fn = extent;
  cmd->extent_id = id;

  if (nbd_internal_run (conn->h, conn, cmd_issue) == -1)
    return -1;

  return cmd->handle;
}
