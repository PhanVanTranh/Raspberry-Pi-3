#include <linux/module.h>
#include <linux/gpio.h>

#define GPIO17 17
#define HIGH 1
#define LOW 0

static int __init gpio_init(void)
{
    gpio_request(GPIO17, "GPIO17");
    gpio_direction_output(GPIO17, 0);
    gpio_set_value(GPIO17, HIGH);
    
    pr_info("GPIO %d initialized and set to HIGH\n", GPIO17);
    return 0;
}

static void __exit gpio_exit(void)
{
    gpio_set_value(GPIO17, LOW);
    gpio_free(GPIO17);

    pr_info("GPIO %d set to LOW and freed\n", GPIO17);
}


module_init(gpio_init);
module_exit(gpio_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TRANH LINUX");
MODULE_DESCRIPTION("A simple GPIO LED driver");
MODULE_VERSION("1.0");