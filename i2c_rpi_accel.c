
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/input.h>


/* Polled input device skeleton */
/* i2cset -y 2 0x1d 0x2d 0x00 */
/* i2cset -y 2 0x1d 0x2d 0x08 */
/* while true; do i2cget -y 2 0x1d 0x32; done */

/* create private structure */
struct ioaccel_dev {
	struct i2c_client * i2c_client;
	struct input_dev * polled_input;
};

#define POWER_CTL	0x2D
#define PCTL_MEASURE	(1 << 3)
#define OUT_X_MSB		0x33

/* poll function */
static void ioaccel_poll(struct input_dev * pl_dev)
{
	struct ioaccel_dev * ioaccel = input_get_drvdata(pl_dev);
	int val = 0;
	val = i2c_smbus_read_byte_data(ioaccel->i2c_client, OUT_X_MSB);


	if ( (val > 0xc0) && (val < 0xff) ) {
		input_event(ioaccel->polled_input, EV_KEY, KEY_1, 1);
	} else {
		input_event(ioaccel->polled_input, EV_KEY, KEY_1, 0);
	}

	input_sync(ioaccel->polled_input);

	dev_info(&ioaccel->i2c_client->dev, "ioaccel_poll() is called.\n");
}

static int ioaccel_probe(struct i2c_client * client,
		const struct i2c_device_id * id)
{
	/* declare an instance of the private structure */
	struct ioaccel_dev * ioaccel;
	int status = 0;

	dev_info(&client->dev, "my_probe() function is called.\n");

	/* allocate private structure for new device */
	ioaccel = devm_kzalloc(&client->dev, sizeof(struct ioaccel_dev), GFP_KERNEL);
	/* Associate client->dev with ioaccel private structure */
	i2c_set_clientdata(client, ioaccel);
	i2c_smbus_write_byte_data(client, POWER_CTL, PCTL_MEASURE);
	
	/* Allocate the struct input_polled_dev */
	ioaccel->polled_input = devm_input_allocate_device(&client->dev);
	
	/* Initialize polled input */
	ioaccel->i2c_client = client;
	
   	input_set_drvdata(ioaccel->polled_input, ioaccel);

	ioaccel->polled_input->dev.parent = &client->dev;
	ioaccel->polled_input->name = "IOACCEL keyboard";
	ioaccel->polled_input->id.bustype = BUS_I2C;

	input_setup_polling(ioaccel->polled_input, ioaccel_poll);
	input_set_poll_interval(ioaccel->polled_input, 500);
    

	/* Set event types */
	set_bit(EV_KEY, ioaccel->polled_input->evbit);
	set_bit(KEY_1, ioaccel->polled_input->keybit);

	/* Register the device, now the device is global until being unregistered*/
	// input_register_polled_device(ioaccel->polled_input);
    status = input_register_device(ioaccel->polled_input);
	dev_info(&client->dev, "input_register_device\n");
	if (status != 0) {
		pr_err("could not register the input_register_device");
		return status;
	}
	return 0;
}

static int ioaccel_remove(struct i2c_client * client)
{
	struct ioaccel_dev * ioaccel;
	ioaccel = i2c_get_clientdata(client);
	input_unregister_device(ioaccel->polled_input);
	dev_info(&client->dev, "ioaccel_remove()\n");
	return 0;
}

/* Add entries to device tree */
static const struct of_device_id ioaccel_dt_ids[] = {
	{ .compatible = "arrow,adxl345", },
	{ }
};
MODULE_DEVICE_TABLE(of, ioaccel_dt_ids);

static const struct i2c_device_id i2c_ids[] = {
	{ "adxl345", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, i2c_ids);

/* create struct i2c_driver */
static struct i2c_driver ioaccel_driver = {
	.driver = {
		.name = "adxl345",
		.owner = THIS_MODULE,
		.of_match_table = ioaccel_dt_ids,
	},
	.probe = ioaccel_probe,
	.remove = ioaccel_remove,
	.id_table = i2c_ids,
};

/* Register to i2c bus as a driver */
module_i2c_driver(ioaccel_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("This is an accelerometer INPUT framework platform driver");
