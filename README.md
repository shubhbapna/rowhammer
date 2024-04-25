# Rowhammer and Rowpress on DDR3

This repository contains code to reverse engineer DRAM address mapping function, execute rowhammer attack and execute rowpress attack.  

## Building the kernels  

There is a Makefile provided to build all the code. Simply run `make` to build all targets. All the executables are created in the `bin` directory at the root of the repository.  

## Reverse engineering DRAM address mapping function  

The code resides in `src/reverse` and `src/histogram`.  

`src/histogram`: It is used to figure on the bank conflict latency. You can run it by executing the following script which also generate a graph for you:  

```bash
$ sudo ./scripts/histogram
```  

`src/reverse`: It is used to detect sets of conflicting addresses using which you can try to estimate a mapping function. Once you have an estimate you can change the code in `src/reverse/verify-dram-mapping.cc` and run that to verify your guess. All of them have corresponding scripts to run them. You can even run the executables directly but the scripts essentially warm up the processor to the defined frequency.  

## Rowhammer  

The code resides in `src/hammer`. When you run the hammer kernel, it will randomly pick address from allocated memory and try to hammer them. If it finds a row where it detected a flip, it will try to confirm whether it is in fact a vulnerable row by hammering it again. The program only exits if a vulnerable row is found (verified or not verified).  

```bash
$ sudo ./bin/hammer | tee hammer.log
```

## Rowpress

The code resides in `src/press`. When you run the press kernel, it will randomly pick address from allocated memory and try to press them. If it finds a row where it detected a flip, it will try to confirm whether it is in fact a vulnerable row by hammering it again. The program keeps running until you kill it.  

To randomly attack addresses run:  

```bash
$ sudo ./bin/press | tee press.log
```  
Once a victim is found, it tries to verify it by pressing the address again. If it produces bit flip then it will add it to a verified.txt file, otherwise it will put it in the non_verified.txt file.  

**Important**: Since it runs forever, if the system is not lightly loaded there changes of swapping occuring which might make the existing PPN to VPN mapping invalid producing segfaults. To over come this you can simply run the kernel from the `scripts` directory.  

To attack specific addresses run (note that between attacking specific addresses it will also attack random addresses):

```bash
$ sudo ./bin/press <physical_victim_address_1> <physical_victim_address_2> <physical_victim_address_3> .... <<physical_victim_address_N>  | tee press.specific.log
```  

