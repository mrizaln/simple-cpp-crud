crud:
	g++ -std=c++20 -g -fsanitize=address main.cpp crud.cpp -o crud

clean:
	rm crud
