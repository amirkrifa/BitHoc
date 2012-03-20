VERSION =	0.2.1

INCLUDE =	-I/usr/local/include/Sockets
CFLAGS =	-Wall -g -O2 $(INCLUDE) -MD `Sockets-config`
CPPFLAGS =	$(CFLAGS)

LIBS =		-L/usr/local/lib -lSockets \
		-lssl -lcrypto -luuid -lSocketsEx -lpthread

PROGS =		newpeer inject
# bstr

all:		$(PROGS)


inject:		MetainfoSocket.o inject.o \
		Session.o bitmap_t.o BString.o PeerHandler.o ParseBT.o Peer.o tSocket.o \
		Request.o cstring.o pSocket.o Piece.o \
		FileManager.o Clock.o
		g++ -o $@ $^ $(LIBS)

newpeer:	Session.o bitmap_t.o newpeer.o BString.o ParseBT.o PeerHandler.o \
		MetainfoSocket.o tSocket.o Peer.o Request.o pSocket.o Piece.o cstring.o \
		FileManager.o \
		StatusSocket.o Clock.o MyLog.o
		g++ -o $@ $^ $(LIBS)

peer:		PeerSocket.o BitTorrentHandler.o peer.o \
		BString.o ParseBT.o FilesManager.o \
		TrackerSocket.o SecureTrackerSocket.o 
		g++ -o $@ $^ $(LIBS)

tracker:	tracker.o TrackerSock.o
		g++ -o $@ $^ $(LIBS)

clean:
		rm -f *.o *.d slask *~ $(PROGS)

-include	*.d

bstr:		bstr.o BString.o ParseBT.o
		g++ -o $@ $^ $(LIBS)


FILES =		BString.h \
		Clock.h \
		FileManager.h \
		MetainfoSocket.h \
		MyLog.h \
		ParseBT.h \
		Peer.h \
		PeerHandler.h \
		Piece.h \
		Request.h \
		Session.h \
		StatusSocket.h \
		bitmap_t.h \
		cstring.h \
		pSocket.h \
		tSocket.h \
		BString.cpp \
		Clock.cpp \
		FileManager.cpp \
		MetainfoSocket.cpp \
		MyLog.cpp \
		ParseBT.cpp \
		Peer.cpp \
		PeerHandler.cpp \
		Piece.cpp \
		Request.cpp \
		Session.cpp \
		StatusSocket.cpp \
		bitmap_t.cpp \
		cstring.cpp \
		inject.cpp \
		newpeer.cpp \
		pSocket.cpp \
		tSocket.cpp \
		Makefile gpl.txt \
		BTDictionary.h \
		BTException.h \
		BTInteger.h \
		BTList.h \
		BTObject.h \
		BTString.h \
		Exception.h color.h \
		Project/*.dsp Project/*.dsw

tar:		clean
		@mkdir -p /usr/local/apache/www.alhem.net/htdocs/project/btpeer
		tar czf btpeer-$(VERSION).tar.gz $(FILES)
		zip -l btpeer-$(VERSION).zip $(FILES)
		/usr/local/bin/tarfix.sh btpeer-$(VERSION)
		cp btpeer-$(VERSION).tar.gz /usr/local/apache/www.alhem.net/htdocs/project/btpeer
		cp btpeer-$(VERSION).zip /usr/local/apache/www.alhem.net/htdocs/project/btpeer

docs:		clean
		rm -rf Rens
		rm -rf Inject
		./mkdot.sh
