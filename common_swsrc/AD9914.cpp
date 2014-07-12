#include "common.h"
#include "fpga.h"
#include "AD9914.h"
#include "dds_pulse.h"
#include <stdio.h>
#include <stdlib.h>


// const double ad9914_clk_MHz = 3500.0;

void set_freq_AD9914PM(void* base_addr, char i, unsigned ftw,
                       unsigned a, unsigned b, FILE* f)
{
    if(f) {
        fprintf(f, "AD9914 board=%i set frequency FTW=0x%08X  A=0x%08X  B=0x%08X\n",
                (unsigned)i, ftw, a, b);
        fflush(f);
    }

    set_dds_four_bytes(base_addr, i, 0x10, ftw);
    set_dds_four_bytes(base_addr, i, 0x14, b);
    set_dds_four_bytes(base_addr, i, 0x18, a);
    set_dds_four_bytes(base_addr, i, 0x2C, ftw);
}

void set_phase_AD9914(void* base_addr, char i, unsigned ptw, FILE* f)
{
    if(f)
        fprintf(f, "AD9914 board=%i set phase %6d / 65535\n", (unsigned)i, ptw);

    PULSER_set_dds_two_bytes(base_addr, i, 0x30, ptw & 0xFFFF);
}

//set amplitude (12 bits, 0...4095)
void set_amp_AD9914(void* base_addr, char i, unsigned A, FILE* f)
{
    if(f)
        fprintf(f, "AD9914 board=%i set amplitude %5d / 4095\n", (unsigned)i, A);

    PULSER_set_dds_two_bytes(base_addr, i, 0x32, A & 0x0FFF);
}

double get_freq_AD9914(void* base_addr, char i) //get freq in Hz
{
    unsigned u0 = PULSER_get_dds_two_bytes(base_addr, i, 0x2C);
    unsigned u1 = PULSER_get_dds_two_bytes(base_addr, i, 0x2E);

    unsigned ftw = u0 | (u1 << 16);
    return FTW2HzD(ftw, AD9914_CLK);
}

unsigned get_ftw_AD9914(void* base_addr, char i) //get freq. tuning word (FTW)
{
    unsigned u0 = PULSER_get_dds_two_bytes(base_addr, i, 0x2C);
    unsigned u1 = PULSER_get_dds_two_bytes(base_addr, i, 0x2E);

    unsigned ftw = u0 | (u1 << 16);
    return ftw;
}

double get_amp_AD9914(void* base_addr, char i) //get amp in % (0...100)
{
    unsigned u0 = PULSER_get_dds_two_bytes(base_addr, i, 0x32);
    return u0/40.95;
}

double get_phase_AD9914(void* base_addr, char i) //get phase in deg (0...360)
{
    unsigned u0 = PULSER_get_dds_two_bytes(base_addr, i, 0x30);
    return u0*360.0/65536.0;
}


bool test_val_AD9914(void* base_addr, char i, unsigned addr, unsigned val)
{
    PULSER_set_dds_two_bytes(base_addr, i, addr, val);

    return PULSER_get_dds_two_bytes(base_addr, i, addr) == val;
}

bool test_AD9914(void* base_addr, char i)
{
    unsigned addr = 0x34; //FTW for profile 1, shouldn't affect the signal
    bool pass = true;

    for(unsigned bit=0; bit<16; bit++) {
        unsigned val = 1 << bit;

        if(!test_val_AD9914(base_addr, i, addr, val)) {
            pass = false;
            fprintf(gLog, "Error on DDS %2d at bit %2d (addr = 0x%02X)\n", (int)i, bit, addr);
        }
    }

    PULSER_set_dds_two_bytes(base_addr, i, addr, 0);

    if(pass)
        fprintf(gLog, "DDS %d passed digital I/O test.\n", (int)i);
    else
        fprintf(gLog, "DDS %d failed digital I/O test.\n", (int)i);

    return pass;
}

void test_dds_addr(void* base_addr, char i, unsigned low_addr,
                   unsigned high_addr, unsigned ntest, FILE* f)
{
    unsigned nerrors = 0;
    unsigned ntested = 0;

    for(unsigned addr=low_addr; addr<=high_addr; addr++) {
        unsigned d0 = PULSER_get_dds_byte(base_addr, i, addr);
        unsigned d1 = PULSER_get_dds_byte(base_addr, i, addr+1);

        for(unsigned j=0; j<ntest; j++) {
            unsigned r = rand() & 0xFF;
            PULSER_set_dds_two_bytes(base_addr, i, addr, r);

            unsigned b = PULSER_get_dds_byte(base_addr, i, addr);

            if( r!= b) {
                fprintf(f, "Error on DDS %d, address 0x%02x.  Wrote 0x%02X.  Read 0x%02X.\n",
                        i, addr, r, b);
                nerrors++;

                if(nerrors > 100)
                    break;
            }

            ntested++;
        }

        PULSER_set_dds_two_bytes(base_addr, i, addr, d0 | (d1 << 8));
    }

    fprintf(f, "Tested %d read/writes in address range 0x%02x to 0x%02x.  %d errors.\n",
            ntested, low_addr, high_addr, nerrors);
}

void print_AD9914_registers(void* base_addr, char i, FILE* f)
{
    fprintf(f, "*********************\n");
    for(unsigned addr=0; (addr+3)<=0x3F; addr+=4) {
        unsigned u0 = PULSER_get_dds_two_bytes(base_addr, i, addr);
        unsigned u2 = PULSER_get_dds_two_bytes(base_addr, i, addr+2);

        fprintf(f, "AD9914 board=%i addr=0x%02X...%02X = %04X%04X\n",
                (unsigned)i, addr+3, addr, u2 & 0xFFFF, u0 & 0xFFFF);
    }

    fprintf(f, "*********************\n");
    fflush(f);
}



void set_dds_four_bytes(void* base_addr, char i, unsigned addr, unsigned data)
{
//	printf("AD9914 board=%i set byte [0x%02X] = 0x%02X\n", addr, data);

    unsigned dds_addr = (addr & 0x3F) << 9; //put addr in bits 14...9 (maps to DDS opcode_reg[14:9] )?
    PULSER_short_pulse(base_addr, 0x1000000F | (i << 4) | (dds_addr << 9), data); // takes 0.3 us
}

void set_freq_AD9914(void* base_addr, char i, double Hz, FILE* f)
{
    //convert Hz to FTW
    unsigned ftw = Hz2FTW(Hz, AD9914_CLK);
    PULSER_set_dds_freq(base_addr, i, ftw);
}

/*
void set_freq_AD9914(void* base_addr, char i, double Hz, bool bPrintInfo)
{
    //convert Hz to FTW, a, and b
    unsigned ftw = static_cast<unsigned int>(floor(0.5 + (Hz * pow(2., 32.) / AD9914_CLK)));
    unsigned b = static_cast<unsigned int>(pow(2., 32.) - 1.);
    unsigned a = static_cast<unsigned int>(floor((Hz * pow(2., 32.) / AD9914_CLK - static_cast<double>(ftw)) * static_cast<double>(b) + 0.5));

    //convert Hz to FTW
    //unsigned ftw = Hz2FTW(Hz, ad9914_clk_MHz);

    unsigned n = gcd(a, b);
    a /= n;
    b /= n;

	if(bPrintInfo)
	{
	    printf("AD9914 board=%i set frequency %9d Hz  FTW=0x%08X  A=0x%08X  B=0x%08X\n", (unsigned)i, (unsigned)Hz, ftw, a, b);
	    fflush(stdout);
	}

  //set_freq_AD9914PM(base_addr, i, ftw, a, b);

  PULSER_set_dds_freq(base_addr, i, ftw);
}
*/
void set_ftw_AD9914(void* base_addr, char i, unsigned ftw, FILE* f)
{

    PULSER_set_dds_two_bytes(base_addr, i, 0x10, ftw & 0xFFFF);
    PULSER_set_dds_two_bytes(base_addr, i, 0x12, (ftw >> 16) & 0xFFFF);

    PULSER_set_dds_two_bytes(base_addr, i, 0x2C, ftw & 0xFFFF);
    PULSER_set_dds_two_bytes(base_addr, i, 0x2E, (ftw >> 16) & 0xFFFF);

    if(f) {
        double Hz = FTW2HzD(ftw, AD9914_CLK);
        fprintf(f, "AD9914 board=%i set frequency %9d Hz  FTW=0x%08X\n",
                (unsigned)i, (unsigned)Hz, ftw);
    }
}

unsigned gcd(unsigned x, unsigned y)
{
    if (y == 0)
        return x; // base case, return x when y equals 0

    return gcd(y,x%y); // recursive call by using arithmetic rules
}

void init_AD9914(void* base_addr, char i)
{
    PULSER_dds_reset(pulser, i);

    //calibrate internal timing.  required at power-up
    PULSER_set_dds_two_bytes(pulser, i, 0x0E, 0x0105);
    usleep(1000U);
    //finish cal. disble sync_out
    PULSER_set_dds_two_bytes(pulser, i, 0x0E, 0x0405);

    //enable programmable modulus and profile 0, enable SYNC_CLK output
    //set_dds_two_bytes(pulser, i, 0x05, 0x8D0B);

    
    //disable programmable modulus, enable profile 0, enable SYNC_CLK output
    //PULSER_set_dds_two_bytes(pulser, i, 0x05, 0x8009);

    //disable ramp & programmable modulus, enable profile 0, disable SYNC_CLK output
    PULSER_set_dds_two_bytes(pulser, i, 0x05, 0x8001);

    //disable programmable modulus and enable profile 0
    //set_dds_byte_AD9914(pulser, i, 0x06, 0x80);

    //enable amplitude control (OSK)
    PULSER_set_dds_two_bytes(pulser, i, 0x0, 0x0008);

    printf("Initialized AD9914 board=%i\n", unsigned(i));
}
