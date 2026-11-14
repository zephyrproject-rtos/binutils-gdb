SECTIONS
{
  zneg = 0xffff8000;
  . = 0x10000;
  .text : { *(.text) }
  /DISCARD/ : { *(*) }
}
