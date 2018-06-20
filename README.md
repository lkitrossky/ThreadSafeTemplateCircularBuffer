

# ThreadSafeTemplateCircularBuffer


**Table of Contents**

[TOCM]

[TOC]

#Why circular buffer
A lot of times we need effective buffer that both written and read. So we organize a block of memory and there are two pointers inside the block: start and stop data. They take almost all block and use modular address arithmetics. Both pointers move periodically, there are several implementations. Usually it is well suited as FIFO buffer.
Usually it indicates that it is full or empty on attempts to write or read.
#Thread safety
IN some implementations there are size of the current data, but this size depends on both operations, and if they are in different threads it is not thread safe.  So, we use only independant pointers.  Because the situation when pointesr are equal is ambigous - empty or full, we avoid this situation taking one more memory element above ordered.

