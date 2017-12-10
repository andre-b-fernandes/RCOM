all: download

download: url.c ip.c url.h ip.h main.c
					gcc ip.c ip.h url.c url.h main.c -o download -Wall

clean:
			-rm -rf download
