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
results?=res.txt
runtime?=time.txt
runtimelines?=5

perfopt=LLC-loads,LLC-load-misses,LLC-stores,LLC-store-misses,LLC-prefetches,L1-dcache-load-misses,L1-dcache-loads,L2-loads,L2-load-misses,branches,branch-misses,branch-load-misses

$(exec): $(exec).cpp
	g++ $(exec).cpp $(flags) -o $(exec) 
	
.PHONY: run
run: $(exec)
	./$(exec) $(file) $(k)

.PHONY: clean
clean:
	@-rm $(exec)

.PHONY: cleanall
cleanall: clean
	@rm kmeans_clusters_*_clusters.png

.PHONY: rerun
rerun: clean run

.PHONY: visualize
visualize: $(file) $(visscript)
	@make $(exec) addflag=-DPRINT
	@./$(exec) $(file) $(k) > $(results)
	$(py) $(visscript) $(results)
	@head -n $(runtimelines) $(results) > $(runtime)
	@cat $(runtime)

.PHONY: vis
vis: visualize
	@open kmeans_clusters_$(k)_clusters.png

.PHONY: prof
prof: $(exec)
	perf stat -e $(perfopt) ./$(exec) $(file) $(k)

.PHONY: ref
ref: baseline.cpp
	@echo "Reference run:"
	@make run exec=baseline
	
.PHONY: speedup
speedup:
	@echo "Calculating speedup for k=$(k)..."
	@make run k=$(k) > run_output.txt
	@make ref k=$(k) > ref_output.txt
