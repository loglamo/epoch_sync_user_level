# README #


### This repo consists of two main missions:
1. Circular buffer: Defined in bounded_buffer.h, trans-sync-v2.c in src folder
2. Montage - (epoch-based synchronization): Defined in 'src/montage' folder


## 1.Circular Buffer with: ##

- 4 main functions: init, free, push, pop
- Access protection with a mutex
- Bounded buffer with two semaphores

### How to use

         gcc -pthread -o [expected_name] trans-sync-v2.c
	     ./expected_name


## 2.MONTAGE: ##

- Explaination of MONTAGE is shown in slide (~/Progress/2023/2023-06-12/progress_2023_06_08_LanAnh.pptx)
- Overview of codes:
![Scheme](https://bitbucket.org/yongseokson/epochsync-userlevel/downloads/btrfs-overview-codes-montage.png)

### How to use

         go to montage folder
		 ./output or recompile with:
		 gcc montage_main_v1.c file_modification_generator.c staging_buffers.c workers.c -o [expected_name]
		 ./[expected_name]
		 
		 

        


