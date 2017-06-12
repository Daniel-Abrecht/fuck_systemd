# fuck_systemd

## What's the problem?

> They don't need to use it, but they can if they want to

Surely it is always a good thing to have more options, but to use systemd isn't. Too many different
things have been merged into systemd, too many things depend on it. Take libsystemd0 for example,
the same library contains, among other things, functions to notify systemd about service state
changes (sd_notify), and functions to log messages using journalctl (sd_journal_send). Those
are some completely unrelated things which belong into different libraries, so they can be replaced
easily. Not to mention, there is a standardized logging function interface available using syslog.h
which is implemented by a lot of logging libraries, and sd_notify is totally useless if all services
are written in a somewhat stable way. And no, a service must be able to handle when another service
is temporarily unavailable, that's nothing a service manager should do, that would only make services
unstable.

> libsystemd0 is a noop without systemd, it won't break or do anything without it

It does break things. I mean, what do you expect to happens when you just remove any functionality from a library?
It's even worse than returning errors, because it just silently fails. Look at the sd_journal_* functions for
example. It replaces syslog which causes any logging service to become incompatible with applications using
this useless new API. Of course, they don't put this functionality it in a distinct library, and they don't
fallback to syslog if systemd is unavailable. In other words: If an application uses sd_journal_* functions for
logging, it won't log anything on systems without systemd!

> libsystemd0 doesn't depend on systemd

No, it isn't. just because it doesn't depend on it yet doesn't mean it won't in the future. Honestly
there are worse things than libsystemd0. Look at systemd-udevd for example, udev is a core linux
service for populating /dev/ and reacting to device changes. Why the hell was it merged into systemd!?!
Sure, we got that into control, with devtempfs, eudev and vdev, we have an escape plan for this. But
this is just the tip of the iceberg. Did you notice that openssh-server in debian stretch depends on
systemd? How could that happen? Over the new systemd-logind dependency of course! PAM isn't good enough
anymore I guess? And that's just one example, I haven't even mentioned things like tors' dependencies
yet...

> There is systemd-shim to substitute systemd, no problem

I don't care, if I don't want systemd, I don't want that shim too. And have you even looked at the
package in debian jessie a little bit closer? You can't compile it without systemd! It's a build
dependency. I'm scared, I have no Idea how many things may need systemd only to be built, and I haven't
noticed yet? Also, what is systemd suddenly decides to drop systemd-shim, surely nobody uses anything
else than systemd? I can't take that risk!

> But there is no good alternative to systemd anyways, only the ugly sysvinit

There are a lot of other really good init systems, which a much smaller and clearer defined scope than
systemd. There are openrc, runit, shepard, upstart, s6, etc. They are all very different, with different
goals. Some do some service supervision, some don't. Some use scripts, some don't. And they all get their
job done reliably, and often even faster than systemd, even when it comes to sysvinit. Sure sysvinit
has it's downsides, but it works well and deterministically. And I can almost hear people screaming
that systemd is deterministic too, and that may be true from a theoretical standpoint, but in practice
starting services asynchronously will always hide some forgotten dependencies, which will skrew up
a system only occasionally. If that's determinism for you, you have a very different definition of
determinism than I have. And by the way, have you seen the diagram about systemd bugs? As of the time
of this writing, the number of open systemd issues always increased, and this is not accounting for
issues closed as "won't fix".

But enough of this, I don't have time to complain about systemd forever.

## What is this all about?

When everything depends on systemd, it's really annoying, if not impossible, to remove all systemd
dependencies from programs. I mean, I appreciate when a program offers an option to build it without
systemd, but it's annoying to do so nonetheless. This is where this library comes into play. An library
or application can link against this library without changing any of it's code and will refuse to start
the application on a systemd system unless configured otherwise. The systemd user can then experience
the pain to recompile the application without fuck-systemd that so many non-systemd users know so well
from systemd.

## How to build

Just install gcc and run make. Just take the fuck_systemd.o file and add it to the linker step of your
program or library. There is also a fuck_systemd-evil.o, which does more evil things than just refusing
to start the program on systemd systems, so maybe don't use that one if you aren't evil.

## Just keep in mind...

You don't need to use it, but you can if you want to.
