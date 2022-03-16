# Analysis of the Kernel oops 
#### oops message upon invoking `echo “hello_world” > /dev/faulty`

```
# echo “hello_world” > /dev/faulty
Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000
Mem abort info:
  ESR = 0x96000046
  EC = 0x25: DABT (current EL), IL = 32 bits
  SET = 0, FnV = 0
  EA = 0, S1PTW = 0
Data abort info:
  ISV = 0, ISS = 0x00000046
  CM = 0, WnR = 1
user pgtable: 4k pages, 39-bit VAs, pgdp=0000000041fc9000
[0000000000000000] pgd=0000000041fcc003, p4d=0000000041fcc003, pud=0000000041fcc003, pmd=0000000000000000
Internal error: Oops: 96000046 [#1] SMP
Modules linked in: faulty(O) scull(O) hello(O)
CPU: 0 PID: 152 Comm: sh Tainted: G           O      5.10.7 #1
Hardware name: linux,dummy-virt (DT)
pstate: 80000005 (Nzcv daif -PAN -UAO -TCO BTYPE=--)
pc : faulty_write+0x10/0x20 [faulty]
lr : vfs_write+0xc0/0x290
sp : ffffffc010bc3db0
x29: ffffffc010bc3db0 x28: ffffff8001ff0c80 
x27: 0000000000000000 x26: 0000000000000000 
x25: 0000000000000000 x24: 0000000000000000 
x23: 0000000000000000 x22: ffffffc010bc3e30 
x21: 00000000004c9940 x20: ffffff8001fa4400 
x19: 0000000000000012 x18: 0000000000000000 
x17: 0000000000000000 x16: 0000000000000000 
x15: 0000000000000000 x14: 0000000000000000 
x13: 0000000000000000 x12: 0000000000000000 
x11: 0000000000000000 x10: 0000000000000000 
x9 : 0000000000000000 x8 : 0000000000000000 
x7 : 0000000000000000 x6 : 0000000000000000 
x5 : ffffff8002225ce8 x4 : ffffffc00867c000 
x3 : ffffffc010bc3e30 x2 : 0000000000000012 
x1 : 0000000000000000 x0 : 0000000000000000 
Call trace:
 faulty_write+0x10/0x20 [faulty]
 ksys_write+0x6c/0x100
 __arm64_sys_write+0x1c/0x30
 el0_svc_common.constprop.0+0x9c/0x1c0
 do_el0_svc+0x70/0x90
 el0_svc+0x14/0x20
 el0_sync_handler+0xb0/0xc0
 el0_sync+0x174/0x180
Code: d2800001 d2800000 d503233f d50323bf (b900003f) 
---[ end trace 3eab66fb7031686e ]---
```

## ERROR Analysis
```
Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000
```
> Trying to deference a NULL pointer Virtual Address 0000000000000000 indicates NULL pointer 


```
Mem abort info:
  ESR = 0x96000046
  EC = 0x25: DABT (current EL), IL = 32 bits
  SET = 0, FnV = 0
  EA = 0, S1PTW = 0
Data abort info:
  ISV = 0, ISS = 0x00000046
  CM = 0, WnR = 1
user pgtable: 4k pages, 39-bit VAs, pgdp=0000000041fc9000
[0000000000000000] pgd=0000000041fcc003, p4d=0000000041fcc003, pud=0000000041fcc003, pmd=0000000000000000
Internal error: Oops: 96000046 [#1] SMP
Modules linked in: faulty(O) scull(O) hello(O)
CPU: 0 PID: 152 Comm: sh Tainted: G           O      5.10.7 #1
Hardware name: linux,dummy-virt (DT)

```
> Segmentation fault occurred as an event of writing to the NULL pointer. The above segment shows different register values of the target system (aarch64) which are set due to this exception. In addition, it also states the modules that are linked to the target image.

```
pc : faulty_write+0x10/0x20 [faulty]
and
Call trace:
 faulty_write+0x10/0x20 [faulty]

```
> This indicates that the memory fault occurred in the faulty module in the faulty_write function at an offset of 0x10 and 32 bytes long

#### Faulty.c the source code for the segmentation fault
```
ssize_t faulty_write (struct file *filp, const char __user *buf, size_t count,
		loff_t *pos)
{
	/* make a simple fault by dereferencing a NULL pointer */
	*(int *)0 = 0;
	return 0;
}
```
