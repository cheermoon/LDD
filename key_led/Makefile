dirs=m_key m_led m_input_key m_plat_led
all:ts modules
ts:key_led.c
	arm-linux-gcc -o ts key_led.c
modules:
	$(foreach N,$(dirs),make -C $(N);)
clean:
	$(foreach N,$(dirs),make -C $(N) clean;)
	rm -f *.ko ts *.o
