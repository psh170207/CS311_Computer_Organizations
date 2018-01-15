cs311sim: cs311.c util.c parse.c run.c
	gcc -g $^ -o $@

.PHONY: clean
clean:
	rm -rf *~ cs311sim

help:
	@echo "The following options are provided with Make\n\t-make:\t\tbuild simulator\n\t-make clean:\tclean the build\n\t-make test:\ttest your simulator"

test: cs311sim test_1 test_2 test_3 test_4 test_5 test_leaf test_beq test_double_loop test_jal test_various_inst

test_1:
	@echo "Testing example01"; \
	./cs311sim -p sample_input/example01.o | diff -Naur sample_output/example01 - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_2:
	@echo "Testing example02"; \
	./cs311sim -p sample_input/example02.o | diff -Naur sample_output/example02 - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_3:
	@echo "Testing example03"; \
	./cs311sim -p sample_input/example03.o | diff -Naur sample_output/example03 - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_4:
	@echo "Testing example04"; \
	./cs311sim -p sample_input/example04.o | diff -Naur sample_output/example04 - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_5:
	@echo "Testing example05"; \
	./cs311sim -p sample_input/example05.o | diff -Naur sample_output/example05 - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_leaf:
	@echo "Testing leaf_example"; \
	./cs311sim -p sample_input/leaf_example.o | diff -Naur sample_output/leaf_example - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_beq:
	@echo "Testing beq_test"; \
	./cs311sim -p sample_input/beq_test.o | diff -Naur sample_output/beq_test - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_double_loop:
	@echo "Testing double_loop"; \
	./cs311sim -p sample_input/double_loop.o | diff -Naur sample_output/double_loop - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_jal:
	@echo "Testing jal_test"; \
	./cs311sim -p sample_input/jal_test.o | diff -Naur sample_output/jal_test - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_various_inst:
	@echo "Testing various_inst"; \
	./cs311sim -p sample_input/various_inst.o | diff -Naur sample_output/various_inst - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi
