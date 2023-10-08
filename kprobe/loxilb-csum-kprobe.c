/*
 *  llb-csum-kprobe.c: LoxiLB checksum fixup probe
 *  Copyright (C) 2023  NetLOX <www.netlox.io>
 * 
 * SPDX-License-Identifier: (GPL-2.0 OR BSD-2-Clause)
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/skbuff.h>

/* For each probe you need to allocate a kprobe structure */
static struct kprobe kp = {
  .symbol_name  = "dev_hard_start_xmit",
};

static int dev_hard_start_xmit_pre(struct kprobe *p, struct pt_regs *regs)
{
  struct sk_buff *skb = (struct sk_buff *)regs->di;

  if (*(__u32 *)&skb->cb[32] == 0xf01dab1f) {
    printk(KERN_INFO "SCTP csum fixup");
    skb->ip_summed = CHECKSUM_NONE;
  }
  return 0;
}

static int __init kprobe_init(void)
{
  int ret;
  kp.pre_handler = dev_hard_start_xmit_pre;

  ret = register_kprobe(&kp);
  if (ret < 0) {
    printk(KERN_INFO "loxilb:kprobe reg failed - %d\n", ret);
    return ret;
  }
  printk(KERN_INFO "loxilb: kprobe at %p\n", kp.addr);
  return 0;
}

static void __exit kprobe_exit(void)
{
  unregister_kprobe(&kp);
  printk(KERN_INFO "loxilb:kprobe at %p unregistered\n", kp.addr);
}

module_init(kprobe_init)
module_exit(kprobe_exit)
MODULE_LICENSE("Dual BSD/GPL");
