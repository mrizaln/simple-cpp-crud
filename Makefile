crud:
	g++ -std=c++17 -g -fsanitize=address -o crud main.cpp crud.cpp

clean:
	rm crud
