# GLCD-BBB-Breakout-Game
1. user_space_game.c contains application which needs to be compiled in user space of BBB and run
2. The lcd_driver.c is the driver code for GLCD which needs to compiled into kernel object and inserted as module into the kernel space
3. The gpio_driver.c is the driver code for GPIO push buttons which needs to compiled into kernel object and inserted as module into the kernel space
