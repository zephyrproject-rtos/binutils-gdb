SECTIONS
{
  .zdata 0x160 : { *(.zdata) }
  . = 0x10000;
  .text : { *(.text) }
  /DISCARD/ : { *(*) }
}
