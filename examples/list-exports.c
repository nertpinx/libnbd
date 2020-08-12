/* This example shows how to list NBD exports.
 *
 * To test this with qemu-nbd:
 *   $ qemu-nbd -x "hello" -t -k /tmp/sock disk.img
 *   $ ./run examples/list-exports /tmp/sock
 *   [0] hello
 *   Which export to connect to (-1 to quit)? 0
 *   Connecting to hello ...
 *   /tmp/sock: hello: size = 2048 bytes
 *
 * To test this with nbdkit (requires 1.22):
 *   $ nbdkit -U /tmp/sock sh - <<\EOF
 *   case $1 in
 *     list_exports) echo NAMES; echo foo; echo foobar ;;
 *     open) echo "$3" ;;
 *     get_size) echo "$2" | wc -c ;;
 *     pread) echo "$2" | dd bs=1 skip=$4 count=$3 ;;
 *     *) exit 2 ;;
 *   esac
 *   EOF
 *   $ ./run examples/list-exports /tmp/sock
 *   [0] foo
 *   [1] foobar
 *   Which export to connect to (-1 to quit)? 1
 *   Connecting to foobar ...
 *   /tmp/sock: foobar: size = 7 bytes
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>

#include <libnbd.h>

int
main (int argc, char *argv[])
{
  struct nbd_handle *nbd;
  int r, i;
  char *name, *desc;
  int64_t size;

  if (argc != 2) {
    fprintf (stderr, "%s socket\n", argv[0]);
    exit (EXIT_FAILURE);
  }

  /* Create the libnbd handle. */
  nbd = nbd_create ();
  if (nbd == NULL) {
    fprintf (stderr, "%s\n", nbd_get_error ());
    exit (EXIT_FAILURE);
  }

  /* Set opt mode and request list exports. */
  nbd_set_opt_mode (nbd, true);
  nbd_set_list_exports (nbd, true);

  /* Connect to the NBD server over a
   * Unix domain socket.  A side effect of
   * connecting is to list the exports.
   * This operation can fail normally, so
   * we need to check the return value and
   * error code.
   */
  r = nbd_connect_unix (nbd, argv[1]);
  if (r == -1 && nbd_get_errno () == ENOTSUP) {
    fprintf (stderr, "%s\n", nbd_get_error ());
    exit (EXIT_FAILURE);
  }
  if (!nbd_aio_is_negotiating (nbd) ||
      nbd_get_nr_list_exports (nbd) == 0) {
    fprintf (stderr, "Server does not support "
             "listing exports.\n");
    exit (EXIT_FAILURE);
  }

  /* Display the list of exports. */
  for (i = 0;
       i < nbd_get_nr_list_exports (nbd);
       i++) {
    name = nbd_get_list_export_name (nbd, i);
    printf ("[%d] %s\n", i, name);
    desc = nbd_get_list_export_description (nbd, i);
    if (desc && *desc)
      printf("  (%s)\n", desc);
    free (name);
    free (desc);
  }
  printf ("Which export to connect to? ");
  if (scanf ("%d", &i) != 1) exit (EXIT_FAILURE);
  if (i == -1) {
    if (nbd_opt_abort (nbd) == -1) {
      fprintf (stderr, "%s\n", nbd_get_error ());
      exit (EXIT_FAILURE);
    }
    nbd_close (nbd);
    exit (EXIT_SUCCESS);
  }
  name = nbd_get_list_export_name (nbd, i);
  if (name == NULL) {
    fprintf (stderr, "%s\n", nbd_get_error ());
    exit (EXIT_FAILURE);
  }
  printf ("Connecting to %s ...\n", name);

  /* Resume connecting to the chosen export. */
  if (nbd_set_export_name (nbd, name) == -1 ||
      nbd_opt_go (nbd) == -1) {
    fprintf (stderr, "%s\n", nbd_get_error ());
    exit (EXIT_FAILURE);
  }
  if (!nbd_aio_is_ready (nbd)) {
    fprintf (stderr, "server closed early\n");
    exit (EXIT_FAILURE);
  }

  /* Read the size in bytes and print it. */
  size = nbd_get_size (nbd);
  if (size == -1) {
    fprintf (stderr, "%s\n", nbd_get_error ());
    exit (EXIT_FAILURE);
  }
  printf ("%s: %s: size = %" PRIi64 " bytes\n",
          argv[1], name, size);

  /* Close the libnbd handle. */
  nbd_close (nbd);

  free (name);

  exit (EXIT_SUCCESS);
}
