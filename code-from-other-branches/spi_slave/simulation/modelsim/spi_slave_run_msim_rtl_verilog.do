transcript on
if {[file exists rtl_work]} {
	vdel -lib rtl_work -all
}
vlib rtl_work
vmap work rtl_work

vlog -vlog01compat -work work +incdir+/home/nik/eie/balance-bug/spi_slave {/home/nik/eie/balance-bug/spi_slave/spi_slave.v}

