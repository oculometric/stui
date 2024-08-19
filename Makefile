BIN			:= bin

CC_FLAGS	:= -I inc -O3 -Wall

$(BIN):
	@mkdir $(BIN)

widgets_demo: $(BIN)
	@g++ -o $(BIN)/widgets_demo $(CC_FLAGS) examples/widgets_demo.cpp
	@$(BIN)/widgets_demo

basic_usage: $(BIN)
	@g++ -o $(BIN)/basic_usage $(CC_FLAGS) examples/basic_usage.cpp
	@$(BIN)/basic_usage

page_usage: $(BIN)
	@g++ -o $(BIN)/page_usage $(CC_FLAGS) examples/page_usage.cpp
	@$(BIN)/page_usage

qr_demo: $(BIN)
	@g++ -o $(BIN)/qr_demo $(CC_FLAGS) examples/qr_demo.cpp
	@$(BIN)/qr_demo
clean:
	rm widgets_demo

.PHONY: clean widgets_demo
