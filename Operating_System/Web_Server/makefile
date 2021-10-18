objects = hashtable.c memory-management.c
test-GD: $(objects)
	gcc -D GD -g -o test $(objects) -lpthread
test-LFU: $(objects)
	gcc -D LFU -g -o test $(objects) -lpthread

test-LRU: $(objects)
	gcc -D LRU -g -o test $(objects) -lpthread

