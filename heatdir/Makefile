# Intel compiler
CC = icx
CFLAGS = -Ofast $(EXTRA_CFLAGS)

MPICC = mpicc

# Job runner function
define job
	@mkdir -p results
	@JOB_ID=$$(sbatch $(1) | awk '{print $$4}'); \
	echo "Submitted batch job $$JOB_ID"; \
	echo "Waiting for job $$JOB_ID to complete..."; \
	while squeue -j $$JOB_ID 2>/dev/null | grep -q $$JOB_ID; do \
		ELAPSED=$$(squeue -j $$JOB_ID --noheader --format="%.10M" 2>/dev/null | tr -d ' '); \
		if [ -n "$$ELAPSED" ]; then \
			printf "\rRunning for: $$ELAPSED"; \
		fi; \
		sleep 1; \
	done; \
	printf "\n"; \
	echo "Job $$JOB_ID completed!"; \
	echo "Results:"; \
	cat results/job-$$JOB_ID.out
endef

all: heat

heat : heat.o input.o misc.o timing.o relax_gauss.o relax_jacobi.o
	$(MPICC) $(CFLAGS) -o $@ $+ -lm 

%.o : %.c heat.h timing.h input.h
	$(MPICC) $(CFLAGS) -c -o $@ $<
	
test : heat
	$(call job,job.test.scp)
	magick heat.ppm heat.jpg
	
micro : heat
	$(call job,job.micro.scp)
	magick heat.ppm heat.jpg
	
local: heat
	mpirun --map-by :OVERSUBSCRIBE -np 6 ./heat test.dat
	magick heat.ppm heat.jpg

clean:
	rm -f *.o heat *~ *.ppm *.jpg *.annot

remake : clean all
