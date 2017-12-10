all: download

download: url.c ip.c url.h ip.h ftp.h ftp.c main.c
					gcc ftp.h ftp.c ip.c ip.h url.c url.h main.c -o download -Wall

clean:
			-rm -rf download
