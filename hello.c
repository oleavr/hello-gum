#define _GNU_SOURCE
#include <fcntl.h>
#include <gum.h>
#include <stdio.h>
#ifdef G_OS_WIN32
# include <windows.h>
#else
# include <dlfcn.h>
# include <unistd.h>
#endif

static void on_enter (GumInvocationContext * ic, gpointer user_data);
static void on_leave (GumInvocationContext * ic, gpointer user_data);

#ifndef G_OS_WIN32
static int (* open_impl) (const char * path, int oflag, ...);
static int replacement_open (const char * path, int oflag, ...);
#endif

int
main (int argc,
      char * argv[])
{
  gum_init ();

  GumInterceptor * interceptor = gum_interceptor_obtain ();

  /* Transactions are optional but improve performance with multiple hooks. */
  gum_interceptor_begin_transaction (interceptor);

#ifdef G_OS_WIN32
  GumInvocationListener * listener =
      gum_make_call_listener (on_enter, on_leave, NULL, NULL);
  gum_interceptor_attach (interceptor, MessageBeep, listener, "MessageBeep");
  gum_interceptor_attach (interceptor, Sleep, listener, "Sleep");
#else
  open_impl = dlsym (RTLD_DEFAULT, "open");

  gum_interceptor_replace (interceptor, open_impl, replacement_open,
      NULL, NULL);
  /*
   * ^
   * |
   * This is using replace(), but there's also attach() which can be used to hook
   * functions without any knowledge of argument types, calling convention, etc.
   * It can even be used to put a probe in the middle of a function.
   *
   * Here's how to use it:
   */
  GumInvocationListener * listener =
      gum_make_call_listener (on_enter, on_leave, NULL, NULL);
  //GumInvocationListener * listener =
  //    gum_make_probe_listener (on_enter, NULL, NULL);
  gum_interceptor_attach (interceptor, open_impl, listener, NULL);
#endif

  gum_interceptor_end_transaction (interceptor);

#ifdef G_OS_WIN32
  MessageBeep (MB_ICONINFORMATION);
  Sleep (1);
#else
  close (open_impl ("/etc/hosts", O_RDONLY));
  close (open_impl ("/etc/fstab", O_RDONLY));
#endif

  return 0;
}

#ifdef G_OS_WIN32

static void
on_enter (GumInvocationContext * ic,
          gpointer user_data)
{
  const char * label = GUM_IC_GET_FUNC_DATA (ic, const char *);
  printf (">>> %s()\n", label);
}

static void
on_leave (GumInvocationContext * ic,
          gpointer user_data)
{
  const char * label = GUM_IC_GET_FUNC_DATA (ic, const char *);
  printf ("<<< %s()\n", label);
}

#else

static int
replacement_open (const char * path,
                  int oflag,
                  ...)
{
  printf ("!!! open(\"%s\", 0x%x)\n", path, oflag);

  return open_impl (path, oflag);
}

static void
on_enter (GumInvocationContext * ic,
          gpointer user_data)
{
  const char * path = gum_invocation_context_get_nth_argument (ic, 0);
  printf (">>> open(path=\"%s\")\n", path);
}

static void
on_leave (GumInvocationContext * ic,
          gpointer user_data)
{
  int fd = GPOINTER_TO_INT (gum_invocation_context_get_return_value (ic));
  printf ("<<< fd=%d\n", fd);
}

#endif
