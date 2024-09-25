#include <clock.h>
#include <console.h>
#include <defs.h>
#include <intr.h>
#include <kdebug.h>
#include <kmonitor.h>
#include <pmm.h>
#include <riscv.h>
#include <stdio.h>
#include <string.h>
#include <trap.h>

// ���Զϵ��쳣�ĺ���
void test_breakpoint() {
    asm volatile("ebreak"); // �⽫����һ���ϵ��쳣
}

void test_illegal_instruction() {
    // ʹ��һ����Ч�Ĳ������������Ƿ�ָ���쳣
    // ����� 0x0067a023 ��һ��ʾ�������룬�������κ���Ч�� RISC-V ָ��
    asm volatile(".4byte 0x1117a023");
}


int kern_init(void) __attribute__((noreturn));
void grade_backtrace(void);

int kern_init(void) {
    extern char edata[], end[];
    memset(edata, 0, end - edata);

    cons_init();  // init the console

    const char *message = "(THU.CST) os is loading ...\n";
    cprintf("%s\n\n", message);

    print_kerninfo();

    // grade_backtrace();

    idt_init();  // init interrupt descriptor table
    // ��ʼ����ɺ󣬵��ò��Ժ���
    cprintf("Testing breakpoint exception... ");
    test_breakpoint(); // �⽫�����ϵ��쳣

    cprintf("Testing illegal instruction exception... ");
    test_illegal_instruction(); // �⽫�����Ƿ�ָ���쳣

    // ������Щ�����ᴥ���쳣����������´��벻��ִ�е�����
    // ����쳣������������������Ȩ�����쳣����󷵻ص�����
    cprintf("This should not be reached.n");
    // rdtime in mbare mode crashes
    clock_init();  // init clock interrupt

    intr_enable();  // enable irq interrupt



    while (1)
        ;
}

void __attribute__((noinline))
grade_backtrace2(unsigned long long arg0, unsigned long long arg1, unsigned long long arg2, unsigned long long arg3) {
    mon_backtrace(0, NULL, NULL);
}

void __attribute__((noinline)) grade_backtrace1(int arg0, int arg1) {
    grade_backtrace2(arg0, (unsigned long long)&arg0, arg1, (unsigned long long)&arg1);
}

void __attribute__((noinline)) grade_backtrace0(int arg0, int arg1, int arg2) {
    grade_backtrace1(arg0, arg2);
}

void grade_backtrace(void) { grade_backtrace0(0, (unsigned long long)kern_init, 0xffff0000); }

static void lab1_print_cur_status(void) {
    static int round = 0;
    round++;
}



