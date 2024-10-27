# Lab 2

## 一、实验要求：

- 基于markdown格式来完成，以文本方式为主
- 填写各个基本练习中要求完成的报告内容
- 列出你认为本实验中重要的知识点，以及与对应的OS原理中的知识点，并简要说明你对二者的含义，关系，差异等方面的理解（也可能出现实验中的知识点没有对应的原理知识点）
- 列出你认为OS原理中很重要，但在实验中没有对应上的知识点

## 二、知识点整理：

### （一）实验中重要的知识点与对应的OS原理
### 1. 物理内存管理
在代码中，物理内存管理的实现通常通过结构体和函数来管理内存页的分配与释放。例如，`struct Page` 代表内存页，每个页面的属性（如是否可用、是否被使用等）由相应的位标志来管理。
#### 代码示例分析
```c
struct Page {
    int property; // 页的属性标志
    struct list_head page_link; // 链接到空闲列表
};
```
这个结构体中的 `property` 用于标识页的状态，确保不同进程对内存的访问互不干扰。

### 2. 虚拟地址与物理地址
在代码中，虚拟地址与物理地址的转换涉及页表的操作。通过查找页表，代码能够将虚拟地址转换为物理地址，并进行访问。

#### 代码示例分析
```c
struct Page *page = le2page(le, page_link); // 从链表节点获取页面
```
这段代码通过链表操作获取一个空闲页面，展示了如何通过结构体和链表管理物理内存。

### 4. 多级页表
代码可能通过多个结构体和指针实现多级页表的概念。在多级页表中，每一层的页表项负责管理更大的地址空间。

#### 代码示例分析
```c
list_add(&free_area[order].free_list, &(buddy->page_link));
```
这行代码表示将一个空闲块加入到特定级别的空闲链表中，反映了多级页表的动态管理。

### 5. 页表项 (PTE) 结构
页表项结构通常在代码中以数组或结构体的形式存在，标记页的权限和状态。

#### 代码示例分析
```c
page->property = n; // 设置页的属性
```
这里，属性包括可读、可写等，直接影响页面的访问权限。

### 6. 页表基址寄存器 (satp)
尽管代码中未直接涉及 `satp`，但可以在页表的初始化和切换中找到相关逻辑。

#### 代码示例分析
```c
ClearPageProperty(page);
```
在进行页表操作前，代码可能需要清除标志，确保系统状态一致性。

### （二）列出你认为OS原理中很重要，但在实验中没有对应上的知识点
建立快表以加快访问效率
#### 练习0：填写已有实验

本实验依赖实验1。请把你做的实验1的代码填入本实验中代码中有“LAB1”的注释相应部分并按照实验手册进行进一步的修改。具体来说，就是跟着实验手册的教程一步步做，然后完成教程后继续完成完成exercise部分的剩余练习。

#### 练习1：理解first-fit 连续物理内存分配算法（思考题）
first-fit 连续物理内存分配算法作为物理内存分配一个很基础的方法，需要同学们理解它的实现过程。请大家仔细阅读实验手册的教程并结合`kern/mm/default_pmm.c`中的相关代码，认真分析default_init，default_init_memmap，default_alloc_pages， default_free_pages等相关函数，并描述程序在进行物理内存分配的过程以及各个函数的作用。
请在实验报告中简要说明你的设计实现过程。请回答如下问题：
- 你的first fit算法是否有进一步的改进空间？
短期优化：可以立即通过按块大小排序的方式优化当前双向链表结构，这能显著提高查找效率，避免地址线性遍历的开销。
长期优化：引入更复杂的数据结构如红黑树、跳表或分离空闲链表结构可以进一步提升内存分配和释放的效率。这些方法可以减少查找和插入空闲块时的时间复杂度，使得内存管理系统更高效、灵活。

1. default_init
"""static void default_init(void) {
    list_init(&free_list);
    nr_free = 0;
}
"""
default_init 函数用于初始化空闲内存块的链表和空闲页的总数：
- list_init(&free_list);：将 free_list 初始化为空的双向链表，用于记录所有空闲内存块。
- nr_free = 0;：将总空闲页数量 nr_free 设置为 0，初始状态没有任何可用的内存页。
此函数确保在开始分配内存之前，系统有一个空的链表和清零的统计值，方便后续管理空闲页。

2. default_init_memmap
   """static void default_init_memmap(struct Page *base, size_t n) {
    assert(n > 0);
    struct Page *p = base;
    for (; p != base + n; p++) {
        assert(PageReserved(p));
        p->flags = p->property = 0;
        set_page_ref(p, 0);
    }
    base->property = n;
    SetPageProperty(base);
    nr_free += n;

    if (list_empty(&free_list)) {
        list_add(&free_list, &(base->page_link));
    } else {
        list_entry_t* le = &free_list;
        while ((le = list_next(le)) != &free_list) {
            struct Page* page = le2page(le, page_link);
            if (base < page) {
                list_add_before(le, &(base->page_link));
                break;
            } else if (list_next(le) == &free_list) {
                list_add(le, &(base->page_link));
            }
        }
    }
}"""
default_init_memmap 函数初始化一个连续的空闲内存块，设置该块中的页面状态并将其加入到 free_list 链表中：
- 页面初始化：循环遍历该块中的每个页面 p，清除它们的标志位和属性（p->flags = 0;），重置引用计数。
- 设置块属性：将该块的第一个页面（base）的 property 设置为整个块的页面数量 n，并设置 PG_property 标志，表示这是一个有效的空闲块。
- 更新总空闲页数：将 n 加入 nr_free 中，更新系统中空闲页的总数。
- 按地址排序插入链表：将 base 页面按地址顺序插入到 free_list 中，这样可以方便后续内存的释放和合并。

3. default_alloc_pages
   """static struct Page *default_alloc_pages(size_t n) {
    assert(n > 0);
    if (n > nr_free) {
        return NULL;
    }
    struct Page *page = NULL;
    list_entry_t *le = &free_list;
    while ((le = list_next(le)) != &free_list) {
        struct Page *p = le2page(le, page_link);
        if (p->property >= n) {
            page = p;
            break;
        }
    }
    if (page != NULL) {
        list_entry_t* prev = list_prev(&(page->page_link));
        list_del(&(page->page_link));
        if (page->property > n) {
            struct Page *p = page + n;
            p->property = page->property - n;
            SetPageProperty(p);
            list_add(prev, &(p->page_link));
        }
        nr_free -= n;
        ClearPageProperty(page);
    }
    return page;
}"""
default_alloc_pages 函数从 free_list 中找到第一个满足请求大小 n 的空闲块并分配内存：
- 寻找空闲块：通过遍历 free_list 中的每个块，找到第一个 property >= n 的块 p。
- 块分配：如果找到合适的块:如果块大小 property 大于请求大小 n，则分割块：分配前 n 个页面给用户，剩余部分重新作为空闲块插入 free_list。

4. default_free_pages
   """static void default_free_pages(struct Page *base, size_t n) {
    assert(n > 0);
    struct Page *p = base;
    for (; p != base + n; p++) {
        assert(!PageReserved(p) && !PageProperty(p));
        p->flags = 0;
        set_page_ref(p, 0);
    }
    base->property = n;
    SetPageProperty(base);
    nr_free += n;

    if (list_empty(&free_list)) {
        list_add(&free_list, &(base->page_link));
    } else {
        list_entry_t* le = &free_list;
        while ((le = list_next(le)) != &free_list) {
            struct Page* page = le2page(le, page_link);
            if (base < page) {
                list_add_before(le, &(base->page_link));
                break;
            } else if (list_next(le) == &free_list) {
                list_add(le, &(base->page_link));
            }
        }
    }

    list_entry_t* le = list_prev(&(base->page_link));
    if (le != &free_list) {
        p = le2page(le, page_link);
        if (p + p->property == base) {
            p->property += base->property;
            ClearPageProperty(base);
            list_del(&(base->page_link));
            base = p;
        }
    }

    le = list_next(&(base->page_link));
    if (le != &free_list) {
        p = le2page(le, page_link);
        if (base + base->property == p) {
            base->property += p->property;
            ClearPageProperty(p);
            list_del(&(p->page_link));
        }
    }
}"""
default_free_pages 函数释放内存，将 n 个页面插入到 free_list 中并合并相邻空闲块：
- 重置页面属性：遍历 base 到 base + n 的页面，将它们的标志位和引用计数重置，并设置 base 的 property 为 n。
- 插入链表：将 base 按地址顺序插入 free_list。
- 合并块：检查 base 前后的块是否为空闲块，如果是，则进行合并，并更新合并后的 property 值。

5. default_nr_free_pages
"""static size_t default_nr_free_pages(void) {
    return nr_free;
}"""
default_nr_free_pages 函数返回当前系统中空闲页面的总数，即 nr_free 的值。

#### 练习2：实现 Best-Fit 连续物理内存分配算法（需要编程）
在完成练习一后，参考kern/mm/default_pmm.c对First Fit算法的实现，编程实现Best Fit页面分配算法，算法的时空复杂度不做要求，能通过测试即可。
请在实验报告中简要说明你的设计实现过程，阐述代码是如何对物理内存进行分配和释放，并回答如下问题：
- 你的 Best-Fit 算法是否有进一步的改进空间？
1. 使用二级或多级分配策略
为了提高内存分配的效率，可以采用二级或多级分配策略。在此策略中，首先根据请求的内存大小选择一个合适的块，如果没有合适的块，则分配一个更大的块并将其拆分为多个小块，以适应不同的请求。这种方法可以减少小块的合并和碎片化问题。
2. 合并空闲块的策略优化
在释放内存时，当前的实现可能需要遍历整个空闲链表来找到可以合并的块。可以考虑在分配时维护一个合并列表，只记录相邻的空闲块，减少查找时间。
#### 扩展练习Challenge：buddy system（伙伴系统）分配算法（需要编程）

Buddy System算法把系统中的可用存储空间划分为存储块(Block)来进行管理, 每个存储块的大小必须是2的n次幂(Pow(2, n)), 即1, 2, 4, 8, 16, 32, 64, 128...

 -  参考[伙伴分配器的一个极简实现](http://coolshell.cn/articles/10427.html)， 在ucore中实现buddy system分配算法，要求有比较充分的测试用例说明实现的正确性，需要有设计文档。
 
#### 扩展练习Challenge：任意大小的内存单元slub分配算法（需要编程）

slub算法，实现两层架构的高效内存单元分配，第一层是基于页大小的内存分配，第二层是在第一层基础上实现基于任意大小的内存分配。可简化实现，能够体现其主体思想即可。

 - 参考[linux的slub分配算法/](http://www.ibm.com/developerworks/cn/linux/l-cn-slub/)，在ucore中实现slub分配算法。要求有比较充分的测试用例说明实现的正确性，需要有设计文档。

#### 扩展练习Challenge：硬件的可用物理内存范围的获取方法（思考题）
  - 如果 OS 无法提前知道当前硬件的可用物理内存范围，请问你有何办法让 OS 获取可用物理内存范围？

1. BIOS/UEFI 固件提供信息
在计算机启动时，操作系统通过 BIOS（基本输入输出系统）或 UEFI（统一可扩展固件接口）获取系统硬件的初始化信息。这是 OS 获取可用物理内存范围的最常见方式之一。
原理：
- BIOS: 在传统的 x86 系统中，BIOS 通过中断调用（如 INT 0x15）为操作系统提供内存映射信息。一个常见的调用是 INT 0x15, EAX=0xE820，这可以返回系统的物理内存布局，包括可用和保留的区域。BIOS 的主要作用是在引导时提供一个基本环境，并让操作系统知道内存的布局信息。
- UEFI: UEFI 是 BIOS 的现代替代品，通过 EFI_MEMORY_DESCRIPTOR 结构体提供更详细的内存映射信息。UEFI 允许 OS 通过 EFI_BOOT_SERVICES.GetMemoryMap() 函数来获取系统中所有内存块的信息，指明哪些块是可用的，哪些是预留给系统或其他用途的。
获取方法：
操作系统在启动时通过调用 BIOS/UEFI 固件接口来获取内存映射信息，然后分析该信息来识别哪些内存块可用。
深入理解：
- 内存映射表：BIOS 或 UEFI 提供的内存映射表包含内存段的开始地址、大小、状态（例如，可用、保留、ACPI 数据等）。操作系统通过读取这些表，确定可用的物理内存范围。
- 内存段标识：内存被标记为不同的类型，例如：
  - RAM：操作系统可以使用的可用内存。
  - Reserved：保留给硬件使用的内存区域，如视频内存或其他设备内存。
  - ACPI 数据：专用于 ACPI 表格的数据。
2. 操作系统启动参数（Bootloader）
引导加载程序（Bootloader）是 OS 启动过程中的一个关键组件，它加载并启动操作系统内核，并传递硬件信息（包括内存范围）。
原理：
- GRUB 等引导加载程序可以通过与 BIOS/UEFI 交互，获取物理内存范围信息并将其传递给操作系统。引导加载程序通常负责在启动阶段收集内存映射表，并将该信息作为启动参数传递给操作系统。
获取方法：
引导加载程序从固件中获取内存映射表，并通过定义好的数据结构（例如 Linux 中的 boot_params 结构体）将该信息传递给操作系统内核。在内核启动时，它会解析这些启动参数以了解物理内存的布局。
深入理解：
引导加载程序是操作系统与 BIOS/UEFI 之间的桥梁。它负责收集硬件相关的信息，并将这些信息作为内核启动时的重要参数。操作系统通过启动参数可以知道内存布局，而无需直接访问 BIOS 或 UEFI。
3. 内存映射与内存扫描
操作系统可以通过直接访问硬件和内存控制器，或者通过硬件寄存器，进行内存扫描来探测可用的物理内存范围。这种方法更加底层，主要用于操作系统无法直接从 BIOS/UEFI 获取内存信息时。
原理：
操作系统可以编写直接访问内存的低级代码，扫描物理地址空间以检测可用内存。例如，操作系统可以通过访问内存控制器、寄存器或使用特定的硬件接口来判断哪些物理地址范围是有效且未使用的。
获取方法：
- 通过遍历某个范围内的物理地址空间，尝试读取和写入内存。
- 操作系统可以向特定物理内存地址写入测试数据，并检查该地址是否返回正确的结果，进而判断该地址是否可用。
深入理解：
- 这种方法需要特别小心，因为直接操作硬件可能会与某些保留内存区域冲突，导致系统崩溃或不稳定。因此，这种方法通常用于某些特殊系统，或者在没有其他信息来源时的最后手段。
- 现代操作系统一般不推荐使用内存扫描，因为 BIOS/UEFI 或 ACPI 已经提供了更安全可靠的内存信息。
4. ACPI（高级配置与电源接口）
ACPI 提供标准化的方法来描述硬件资源，操作系统可以通过 ACPI 表格（例如 SRAT, E820 表格）获取关于物理内存的信息。
原理：
ACPI 是一种标准化接口，允许操作系统获取关于硬件配置的详细信息，包括内存布局、电源管理、设备控制等。ACPI 表格存储了系统内存的详细布局信息。
获取方法：
操作系统在启动过程中，加载并解析 ACPI 表格，尤其是 E820 和 SRAT（系统资源分配表），以获取系统中可用的物理内存范围。
深入理解：
- ACPI 表格 通常包含物理内存的分段信息，包括哪些内存可以用，哪些内存被设备或固件保留。通过这些表格，操作系统可以准确判断哪些物理内存是可用的。
- ACPI 还提供了一些电源管理和热插拔设备支持，这些特性使操作系统能够在硬件状态变化时动态调整可用内存范围。
5. 硬件抽象层（HAL）
硬件抽象层（HAL）在某些操作系统中用于屏蔽底层硬件的复杂性。操作系统通过 HAL 获取和管理硬件资源（如内存），而 HAL 可以依赖 BIOS/UEFI 或硬件寄存器来确定物理内存的布局。
原理：
- HAL 通过提供一个统一的接口，使操作系统可以与不同的硬件平台交互，而无需为每个平台编写专门的代码。在获取物理内存布局时，HAL 可能依赖于 BIOS/UEFI 或 ACPI，作为中间层进行信息的传递。
获取方法：
操作系统通过调用 HAL 提供的 API 函数来获取硬件信息，尤其是物理内存的可用范围。这些 API 通过底层固件或者硬件寄存器与实际硬件进行交互。
深入理解：
- HAL 的作用是屏蔽硬件差异，提供一个标准化的硬件接口。
- HAL 本身并不直接生成物理内存信息，而是通过底层硬件或固件获取该信息，然后传递给操作系统。
6. 处理器支持的机制
某些处理器架构本身提供内存探测机制，允许操作系统通过特定指令集或寄存器获取物理内存的布局。这种机制因处理器架构不同而有所差异。
获取方法：
操作系统可以通过读取处理器的特定寄存器或者通过处理器提供的指令集接口来获取内存布局。例如，在 x86 系统中，可以通过 CPUID 指令获取某些系统资源信息。

> Challenges是选做，完成Challenge的同学可单独提交Challenge。完成得好的同学可获得最终考试成绩的加分。
