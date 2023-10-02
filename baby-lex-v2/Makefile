cc := clang

src 	 := $(shell find . -wholename "./src/*.c")
obj 	 := $(patsubst ./src/%.c, ./build/obj/%.o, $(src))
cc_ops := -I./include/

all: $(obj)
	$(cc) $^ -o ./build/exec -g

$(obj): build/obj/%.o: src/%.c | dir_init
	$(cc) $(cc_ops) -c $< -o $@ -g

dir_init:
	mkdir -p ./build/obj/
