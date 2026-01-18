UNAME_S != uname -s
OS = ${UNAME_S}

.if ${UNAME_S} == "OpenBSD"
OS = openbsd
.elif ${UNAME_S} == "FreeBSD"
OS = freebsd
.endif

NAME != cat main.c | grep "const char \*sofname" | awk '{print $$5}' |\
	sed "s/\"//g" | sed "s/;//"
VERSION != cat main.c | grep "const char \*version" | awk '{print $$5}' |\
	sed "s/\"//g" | sed "s/;//"

PREFIX = /usr/local

# cc | clang | gcc
CC = cc
IMPLEMENTS = -DSUWAUI_IMPLEMENTS_SUWAWINDOW \
	     -DSUWAUI_IMPLEMENTS_SUWALABEL \
	     -DSUWAUI_IMPLEMENTS_SUWABUTTON
# lldb | gdb
DEBUGGER = lldb
FILES = *.c

CFLAGS = -Wall -Wextra \
	 -I./include -I/usr/include -I/usr/local/include -I/usr/X11R6/include \
	 -L/usr/lib -L/usr/local/lib -L/usr/X11R6/lib \
	 -I/usr/X11R6/include/freetype2 -I/usr/local/include/freetype2

LDFLAGS = -lc -lX11 -lXft
.if ${UNAME_S} == "FreeBSD"
LDFLAGS += -lsys
.endif
.if ${UNAME_S} == "FreeBSD"
SLIB += -lxcb -lthr -lfontconfig -lfreetype -lXrender -lXau -lXdmcp -lexpat -lintl \
        -lbz2 -lpng16 -lbrotlidec -lz -lm -lbrotlicommon
.elif ${UNAME_S} == "OpenBSD"
SLIB += -lxcb -lpthread -lfontconfig -lz -lexpat -lfreetype -lXrender -lXau -lXdmcp
.endif

all: debug

debug:
	${CC} ${IMPLEMENTS} -O0 -g ${CFLAGS} -o ${NAME} ${FILES} ${LDFLAGS}
	${DEBUGGER} -o run ${NAME}

develop:
	${CC} ${IMPLEMENTS} -O3 -g ${CFLAGS} -o ${NAME} ${FILES} ${LDFLAGS} ${SLIB}
	./${NAME}

release:
	${CC} ${IMPLEMENTS} -O3 ${CFLAGS} -o ${NAME} ${FILES} -static ${LDFLAGS} ${SLIB}
	strip ${NAME}
	./${NAME}

clean:
	rm -rf ${NAME} *.core *.o

install:
	cp ${NAME} ${PREFIX}/bin
	chmod +x ${PREFIX}/bin/${NAME}

uninstall:
	rm -rf ${PREFIX}/bin/${NAME}

dist:

.PHONY: all debug develop release clean install uninstall dist
