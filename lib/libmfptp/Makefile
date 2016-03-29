object =   mfptp_parser.c mfptp_utils.c mfptp_pack.c
     
LIBS =-lssl -lcrypto -lz 

libmfptp: $(object)
	gcc  $(object) $(LIBS) -fPIC -shared -o libmfptp.so

clean:
	rm libmfptp.so
