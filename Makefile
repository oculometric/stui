widgets_demo:
	g++ -o widgets_demo examples/widgets_demo.cpp -I ./ -O0

clean:
	rm widgets_demo

.PHONY: clean widgets_demo