# Thank you, FreeBSD, for making this &@^%ing kluge necessary.
# We can all tell how dedicated you are to making things work out of the box.
# Perhaps you'd like to relocate 'sh' as well?  How about
#  /usr/shells/bin/sh ?  or perhaps /sys/bin/sh ?
if [ -z "$BASH" ]; then
	exec bash $0
fi

