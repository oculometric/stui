BIN			:= bin

CC_FLAGS	:= -I inc -O3 -Wall

$(BIN):
	@mkdir $(BIN)

widgets_demo: $(BIN)
	@g++ -o $(BIN)/widgets_demo $(CC_FLAGS) examples/widgets_demo.cpp
	@$(BIN)/widgets_demo

doxygen:
	@echo "generate the doxygen docs!"

clean:
	rm widgets_demo

.PHONY: clean widgets_demo