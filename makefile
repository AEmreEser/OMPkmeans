# A. Emre Eser - 2024
#
k?=4
file?=input.txt
opt?=3
addflag?=
flags:=-O$(opt) -fopenmp $(addflag)
exec?=kmeans
visscript?=visualize.py
py?=python3
results?= $(basename $(file))_res.txt
testres?=$(basename $(file))_test_res.txt
runtime?=time.txt
runtimelines?=5
tend?=10
SHELL := /bin/bash

perfopt=LLC-loads,LLC-load-misses,LLC-stores,LLC-store-misses,LLC-prefetches,L1-dcache-load-misses,L1-dcache-loads,L2-loads,L2-load-misses,branches,branch-misses,branch-load-misses

$(exec): $(exec).cpp
	g++ $(exec).cpp $(flags) -o $(exec) 

baseline: baseline.cpp
	g++ baseline.cpp $(flags) $(addflag) -o baseline 
	
.PHONY: run
run: $(exec)
	./$(exec) $(file) $(k)

.PHONY: clean
clean:
	-@rm $(exec)
	-@rm baseline

.PHONY: cleanall
cleanall: clean
	-@rm kmeans_clusters_*_clusters.png
	-@rm *_res.txt

.PHONY: rerun
rerun: clean run

.PHONY: visualize
visualize: $(file) $(visscript)
	make $(exec) addflag=-DPRINT
	./$(exec) $(file) $(k) > $(results)
	$(py) $(visscript) $(results)
	@head -n $(runtimelines) $(results) > $(runtime)
	@cat $(runtime)

.PHONY: vis
vis: visualize
	@open kmeans_clusters_$(k)_clusters.png &

.PHONY: prof
prof: $(exec)
	perf stat -e $(perfopt) ./$(exec) $(file) $(k)

.PHONY: ref
ref: baseline
	@echo "Reference run:"
	./baseline $(file) $(k)

.PHONY: runcmd
runcmd: $(exec) baseline
	@echo "=========$(k)=========="
	make run k=$(k)
	@echo "=========$(k)=========="
	make ref k=$(k)

.PHONY: test
test: $(exec) baseline
	for i in $(shell seq 0 $(tend)); do k=$$((2 ** $$i)); make runcmd k=$$k; done 

.PHONY: testrec
testrec: 
	make test tend=$(tend) file=$(file) > $(testres)
	
.PHONY: speedup
speedup:
	@echo "Calculating speedup for k=$(k)..."
	make run k=$(k) > run_output.txt
	make ref k=$(k) > ref_output.txt
