api_test:api_client_test.c
	gcc -o api_test \
		-I$(E3_SDK)/e3api/include \
		-I$(E3_SDK)/e3api_export/libs/ \
		-le3api -lzmq -lpthread \
		-L$(E3_SDK)/e3api_export/libs  api_client_test.c


clean:
	rm -f api_test
