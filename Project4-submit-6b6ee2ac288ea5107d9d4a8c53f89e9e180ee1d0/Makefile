cs311sim: cs311.c util.c parse.c run.c cache.c linked_queue.c
	gcc -g -O2 $^ -o $@

.PHONY: clean
clean:
	rm -rf *~ cs311sim

help:
	@echo "The following options are provided with Make\n\t-make:\t\tbuild simulator\n\t-make clean:\tclean the build\n\t-make test:\ttest your simulator"

test: cs311sim test_leaf test_various_inst test_str_cpy test_add_vec test_trans_mat


test_leaf:
	@echo "Testing leaf_example"; \
        ./cs311sim -x -p grading_input/leaf_example.o | diff -Naur grading_output/leaf_example - ;\
        if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_various_inst:
	@echo "Testing various_inst"; \
        ./cs311sim -x -p grading_input/various_inst.o | diff -Naur grading_output/various_inst - ;\
        if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_str_cpy:
	@echo "Testing str_cpy"; \
	./cs311sim -x -p -n 150 grading_input/str_cpy.o | diff -Naur grading_output/str_cpy - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi


test_add_vec:
	@echo "Testing add_vec"; \
	./cs311sim -x -p -n 200 grading_input/add_vec.o | diff -Naur grading_output/add_vec - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi
test_trans_mat:
	@echo "Testing trans_mat"; \
	./cs311sim -x -p -n 200 grading_input/trans_mat.o | diff -Naur grading_output/trans_mat - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi


