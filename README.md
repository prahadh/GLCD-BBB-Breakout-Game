# GLCD-BBB-Breakout-Game
1. user_space_game.c contains application c code which needs to be compiled in user space of BBB and run
2. The lcd_driver.c is the driver c code for GLCD which needs to compiled into kernel object and inserted as module into the kernel space of BBB
3. The gpio_driver.c is the driver c code for GPIO push buttons which needs to compiled into kernel object and inserted as module into the kernel space of BBB
