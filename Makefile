all: libgunrock py

py:
	python example.py

libgunrock:
	g++ -std=c++11 -c -fPIC lambda.cpp -o gunrock.o
	g++ -shared -Wl,-install_name,libgunrock.so -o libgunrock.so gunrock.o