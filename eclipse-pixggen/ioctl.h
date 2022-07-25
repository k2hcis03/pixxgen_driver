#ifndef IOCTL_H_
#define IOCTL_H_
 
struct __attribute__((packed)) ioctl_info {
	u8 exout[8];
	u8 exin[3];
	u8 gpio_i2c_out_a;
	u8 gpio_i2c_out_b;
	u8 gpio_i2c_in_a;
	u8 gpio_i2c_in_b;
	u8 pot_i2c_write;
	u32 pot_i2c_read;
	u8 collimator_mux_i2c_write;			//@k2h I2C 먹스 설정	0x04 = 0, 0x05 = 1
	u8 collimator_mux_i2c_read;				//@k2h I2C 먹스 설정	0x04 = 0, 0x05 = 1
	u8 adc_start;
	u32 st_motor_count;
	u32 st_motor_speed;
	u8 collimator_motor_number;
	s32 collimator_motor_count;			//motor move count
	u8 collimator_motor_sensor;
	u8 collimator_motor_acc;					//가속도 테이블 사용유
	u8 collimator_motor_speed;				//가속도 테이블에 곱해지는 값 --> 속도를 줄이는데 사용
	u8 collimator_motor_sleep;
	u8 collimator_motor_mode;
	u8 collimator_motor_laser;
	u8 collimator_motor_res;
	u32 collimator_motor_status;			//motor sensor status bit0->motor1 L limit bit1-> motor1 R limit....
};

#define             	IOCTL_MAGIC         	'R'
#define             	SET_GPIO            	_IOW(IOCTL_MAGIC, 2, struct ioctl_info )
#define             	GET_GPIO            	_IOR(IOCTL_MAGIC, 3, struct ioctl_info )
#define				SET_I2C_GPIO_A		_IOW(IOCTL_MAGIC, 4, struct ioctl_info )
#define				SET_I2C_GPIO_B		_IOW(IOCTL_MAGIC, 5, struct ioctl_info )
#define				GET_I2C_GPIO_A		_IOR(IOCTL_MAGIC, 6, struct ioctl_info )
#define				GET_I2C_GPIO_B		_IOR(IOCTL_MAGIC, 7, struct ioctl_info )
#define				SET_I2C_POT			_IOW(IOCTL_MAGIC, 8, struct ioctl_info )
#define				GET_I2C_POT			_IOR(IOCTL_MAGIC, 9, struct ioctl_info )
#define				SET_COLLIMATOR_MUX	_IOW(IOCTL_MAGIC, 10, struct ioctl_info )
#define				SET_COLLIMATOR		_IOW(IOCTL_MAGIC, 11, struct ioctl_info )
#define				GET_COLLIMATOR		_IOR(IOCTL_MAGIC, 12, struct ioctl_info )
//#define				SET_COLLIMATOR_2		_IOW(IOCTL_MAGIC, 13, struct ioctl_info )
//#define				GET_COLLIMATOR_2		_IOR(IOCTL_MAGIC, 14, struct ioctl_info )
#define				ADC_START				_IOW(IOCTL_MAGIC, 15, struct ioctl_info )
#define				ADC_STOP				_IOW(IOCTL_MAGIC, 16, struct ioctl_info )
#define				START_STEP_MOTOR1		_IOW(IOCTL_MAGIC, 17, struct ioctl_info )
#define				START_STEP_MOTOR2		_IOW(IOCTL_MAGIC, 18, struct ioctl_info )
#define				START_STEP_MOTOR3		_IOW(IOCTL_MAGIC, 19, struct ioctl_info )
#define				START_STEP_MOTOR4		_IOW(IOCTL_MAGIC, 20, struct ioctl_info )
#define				STOP_STEP_MOTOR1		_IOW(IOCTL_MAGIC, 21, struct ioctl_info )
#define				STOP_STEP_MOTOR2		_IOW(IOCTL_MAGIC, 22, struct ioctl_info )
#define				STOP_STEP_MOTOR3		_IOW(IOCTL_MAGIC, 23, struct ioctl_info )
#define				STOP_STEP_MOTOR4		_IOW(IOCTL_MAGIC, 24, struct ioctl_info )
#define				START_DC_MOTOR1		_IOW(IOCTL_MAGIC, 25, struct ioctl_info )
#define				START_DC_MOTOR2		_IOW(IOCTL_MAGIC, 26, struct ioctl_info )
#define				START_DC_MOTOR3		_IOW(IOCTL_MAGIC, 27, struct ioctl_info )
#define				STOP_DC_MOTOR1		_IOW(IOCTL_MAGIC, 28, struct ioctl_info )
#define				STOP_DC_MOTOR2		_IOW(IOCTL_MAGIC, 29, struct ioctl_info )
#define				STOP_DC_MOTOR3		_IOW(IOCTL_MAGIC, 30, struct ioctl_info )

//#define				START_COLL_MOTOR1		_IOW(IOCTL_MAGIC, 31, struct ioctl_info )
//#define				START_COLL_MOTOR2		_IOW(IOCTL_MAGIC, 32, struct ioctl_info )
//#define				START_COLL_MOTOR3		_IOW(IOCTL_MAGIC, 33, struct ioctl_info )
//#define				START_COLL_MOTOR4		_IOW(IOCTL_MAGIC, 34, struct ioctl_info )
//#define				START_COLL_MOTOR5		_IOW(IOCTL_MAGIC, 35, struct ioctl_info )
//#define				STOP_COLL_MOTOR1		_IOW(IOCTL_MAGIC, 36, struct ioctl_info )
//#define				STOP_COLL_MOTOR2		_IOW(IOCTL_MAGIC, 37, struct ioctl_info )
//#define				STOP_COLL_MOTOR3		_IOW(IOCTL_MAGIC, 38, struct ioctl_info )
//#define				STOP_COLL_MOTOR4		_IOW(IOCTL_MAGIC, 39, struct ioctl_info )
//#define				STOP_COLL_MOTOR5		_IOW(IOCTL_MAGIC, 40, struct ioctl_info )

#endif /* IOCTL_H_ */
