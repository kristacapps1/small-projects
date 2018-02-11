target:
	g++ -lm -o mgetweb mgetweb.cpp -lpthread -std=c++11
	g++ -lm -o multithreaded_copy_file_local multithreaded_copy_file_local.cpp -lpthread -std=c++11
	g++ -lm -o time_file_objects time_file_objects.cpp -lpthread -std=c++11
