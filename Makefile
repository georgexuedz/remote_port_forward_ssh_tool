ifeq ($(ver), debug)
    CXXFLAGS = -D __DEBUG__
else
    CXXFLAGS = 
endif


multi_client: multi_process_tcp_forward.c multi_process_tcp_forward.h jl_debug.h jl_buffer.o jl_libssh2.o tcpip_forward.o jl_json.o
	gcc $(CXXFLAGS) -o nc2rctrl tcpip_forward.o  multi_process_tcp_forward.c jl_json.o jl_buffer.o jl_libssh2.o  -L /usr/local/lib/ -lssh2 cJSON-master/cJSON.c -lm 

jl_json.o: jl_json.h jl_json.c
	gcc $(CXXFLAGS) -c jl_json.c
    
jl_buffer.o:  jl_buffer.h jl_buffer.c
	gcc $(CXXFLAGS) -c jl_buffer.c

jl_libssh2.o:  jl_libssh2.h jl_libssh2.c jl_json.o jl_buffer.c jl_debug.h
	gcc $(CXXFLAGS) -c jl_libssh2.c jl_buffer.c

tcpip_forward.o: tcpip_forward.c tcpip_forward.h jl_json.o jl_libssh2.o jl_buffer.o jl_debug.h
	gcc $(CXXFLAGS) -c jl_json.c tcpip_forward.c


.PHONY: clean

clean:
	rm -f *.o
	rm -f *.gch

