#include "thermal.h"

void thermalInit(void)
{
    if (HAL_I2C_IsDeviceReady(&hi2c1, DEV_ADDR, 3, TIMEOUT) == HAL_OK)
    {
        uartPrintf(0, "mlx detected!!\r\n");
        
        uint16_t status = 0;
        uint8_t refresh_rate = 4;

        if (thermalReadRegister(0x8000, &status) == true)
        {
            uartPrintf(0, "STATUS = 0x%04X\r\n", status);
        }

        if (thermalSetRefreshRate(MLX90640_REFRESH_16HZ) == true)
        {
            uartPrintf(0, "MLX Refresh Rate Set Compelted\r\n");
        }
        else
        {
            uartPrintf(0, "MLX Refresh Rate set failed\r\n");
        }
    }
    else 
    {
        uartPrintf(0, " MLX Detect Error!!\r\n");
    }
}

bool thermalReadRegister(uint16_t reg_addr, uint16_t *data)
{
    uint8_t buf[2]; 
    
    if (HAL_I2C_Mem_Read(&hi2c1, DEV_ADDR, reg_addr, I2C_MEMADD_SIZE_16BIT, buf, 2, TIMEOUT) == HAL_OK)
    {
        *data = ((uint16_t)buf[0] << 8) | buf[1];
    }
    else 
    {
        return false;
    }

    return true;
}

bool thermalWriteRegister(uint16_t reg_addr, uint16_t data)
{
    uint8_t buf[2];

    buf[0] = (data >> 8) & 0xFF;
    buf[1] = data & 0xFF;

    if (HAL_I2C_Mem_Write(&hi2c1, DEV_ADDR, reg_addr, I2C_MEMADD_SIZE_16BIT, buf, 2, TIMEOUT) == HAL_OK)
    {
        return true;
    }
    else 
    {
        return false;
    }
}

bool thermalReadFrame(uint16_t *frame_buf)
{
    uint16_t i;
    uint16_t data;
    uint16_t addr = MLX90640_RAM_START_ADDR; 

    for (i = 0; i < MLX90640_FRAME_WORD_COUNT; i++)
    {
        if (thermalReadRegister(addr, &data) == false)
        {
            return false;
        }

        frame_buf[i] = data;
        addr++;
    }

    return true;
}

bool thermalReadCalibData(uint16_t *eeprom_buf)
{
    uint16_t addr = MLX90640_EEPROM_START_ADDR;
    uint16_t i;
    uint16_t data;

    for (i = 0; i < MLX90640_EEPROM_WORD_COUNT; i++)
    {
        if (thermalReadRegister(addr, &data) == false)
        {
            cliPrintf("EEPROM read fail at i=%d addr=0x%04X\r\n", i, addr);
            return false;
        }

        eeprom_buf[i] = data;
        addr++; 
    }

    return true; 
}

bool thermalWaitNewData(uint16_t *status)
{
    while (1) 
    {
        if (thermalReadRegister(MLX90640_STATUS_REG, status) == false)
        {
            return false;
        }

        if ((*status & MLX90640_NEW_DATA_BIT) != 0)
        {
            return true;
        }

        osDelay(5);
    }
}

uint8_t thermalGetSubpage(uint16_t status)
{
    if ((status & MLX90640_SUBPAGE_BIT) == 0)
    {
        return 0;
    }
    else
    {
        return 1;
    } 
}

bool thermalClearNewDataFlag(void)
{
    uint16_t status;

    if (thermalReadRegister(MLX90640_STATUS_REG, &status) == false)
    {
        return false;
    }

    status &= ~MLX90640_NEW_DATA_BIT;

    if (thermalWriteRegister(MLX90640_STATUS_REG, status) == false)
    {
        return false;
    }

    return true;
}

bool thermalReadFrameSynced(uint16_t *frame_buf, uint8_t *subpage)
{
    uint16_t status = 0;
    uint32_t timeout = 100;

    if (frame_buf == NULL || subpage == NULL)
    {
        return false;
    }

    while (timeout--)
    {
        if (thermalReadRegister(MLX90640_STATUS_REG, &status) == false)
        {
            return false;
        }

        if ((status & MLX90640_NEW_DATA_BIT) != 0)
        {
            break;
        }

        HAL_Delay(1); // osDelay
    }

    if ((status & MLX90640_NEW_DATA_BIT) == 0)
    {
        return false;
    }

    *subpage = thermalGetSubpage(status);

    if (thermalReadFrame(frame_buf) == false)
    {
        return false;
    }

    if (thermalClearNewDataFlag() == false)
    {
        return false;
    }

    return true;
}

static int16_t thermalToSigned16(uint16_t data)
{
    if (data > 32767)
    {
        return (int16_t)(data - 65536);
    }

    return (int16_t)data;
}

bool thermalExtractParameters(uint16_t *eeprom_buf, thermal_params_t *params)
{
    if (eeprom_buf == NULL || params == NULL)
    {
        return false;
    }

    memset(params, 0, sizeof(thermal_params_t));

    int16_t kVdd = (eeprom_buf[51] & 0xFF00) >> 8;

    if (kVdd > 127)
    {
        kVdd -= 256;
    }

    kVdd *= 32;

    int16_t vdd25 = eeprom_buf[51] & 0x00FF;
    vdd25 = ((vdd25 - 256) << 5) - 8192;

    params->kVdd = kVdd;
    params->vdd25 = vdd25;

    int16_t KvPTAT = (eeprom_buf[50] & 0xFC00) >> 10;

    if (KvPTAT > 31)
    {
        KvPTAT -= 64;
    }

    params->kvPTAT = (float)KvPTAT / 4096.0f;

    int16_t KtPTAT = eeprom_buf[50] & 0x03FF;

    if (KtPTAT > 511)
    {
        KtPTAT -= 1024;
    }

    params->ktPTAT = (float)KtPTAT / 8.0f;

    params->vPTAT25 = thermalToSigned16(eeprom_buf[49]);

    params->alphaPTAT = (float)((eeprom_buf[16] & 0xF000) >> 12);
    params->alphaPTAT = params->alphaPTAT / 4.0f + 8.0f;

    params->resolutionEE = (eeprom_buf[56] & 0x3000) >> 12;

    return true;
}

float thermalCalculateVdd(uint16_t *frame_buf, thermal_params_t *params)
{
    if (frame_buf == NULL || params == NULL)
    {
        return 0.0f;
    }

    int16_t vdd_raw = thermalToSigned16(frame_buf[810]);

    uint8_t resolutionRAM = (frame_buf[832] & 0x0C00) >> 10;

    float resolutionCorrection;

    resolutionCorrection = powf(2.0f, (float)params->resolutionEE) /
                           powf(2.0f, (float)resolutionRAM);

    float vdd;

    vdd = (resolutionCorrection * (float)vdd_raw - (float)params->vdd25) /
          (float)params->kVdd + 3.3f;

    return vdd;
}

float thermalCalculateTa(uint16_t *frame_buf, thermal_params_t *params)
{
    if (frame_buf == NULL || params == NULL)
    {
        return 0.0f;
    }

    float vdd = thermalCalculateVdd(frame_buf, params);

    int16_t ptat_raw = thermalToSigned16(frame_buf[800]);
    int16_t ptat_art_raw = thermalToSigned16(frame_buf[768]);

    float ptat = (float)ptat_raw;
    float ptatArt = (float)ptat_art_raw;

    ptatArt = (ptat / (ptat * params->alphaPTAT + ptatArt)) * powf(2.0f, 18.0f);

    float ta;

    ta = ptatArt / (1.0f + params->kvPTAT * (vdd - 3.3f));
    ta = (ta - (float)params->vPTAT25) / params->ktPTAT + 25.0f;

    return ta;
}

bool thermalCalculateTemperature(uint16_t *frame_buf,
                                 thermal_params_t *params,
                                 float emissivity,
                                 float reflected_temp,
                                 float *temperature_buf)
{
    if (frame_buf == NULL || params == NULL || temperature_buf == NULL)
    {
        return false;
    }

    if (emissivity <= 0.0f || emissivity > 1.0f)
    {
        emissivity = 1.0f;
    }

    float ta = thermalCalculateTa(frame_buf, params);

    for (int i = 0; i < MLX90640_PIXEL_COUNT; i++)
    {
        int16_t ir_raw = thermalToSigned16(frame_buf[i]);

        float temp;

        temp = ta + ((float)ir_raw * 0.01f);

        temp = (temp * emissivity) + (reflected_temp * (1.0f - emissivity));

        temperature_buf[i] = temp;
    }

    return true;
}

void thermalPrintRawFrame(void)
{
    static uint16_t frame_buf[MLX90640_FRAME_WORD_COUNT];

    if (thermalReadFrame(frame_buf) == true)
    {
        uint16_t i;

        for (i = 0; i < MLX90640_PIXEL_COUNT; i++)
        {
            uartPrintf(0, "[%03d] 0x%04x\r\n", i, frame_buf[i]);
        }
    }
    else 
    {
        uartPrintf(0, "frame read failed...\r\n");
    }
}

void cliThermal(uint8_t argc, char *argv[])
{
    static uint16_t frame_buf[MLX90640_FRAME_WORD_COUNT];
    static uint16_t eeprom_buf[MLX90640_EEPROM_WORD_COUNT];
    static float temperature_buf[MLX90640_PIXEL_COUNT];
    static thermal_params_t params;

    uint8_t subpage = 0;
    float ta;
    float reflected_temp;

    if (argc == 2 && strcmp(argv[1], "tem") == 0)
    {
        cliPrintf("MLX90640 temperature read...\r\n");

        if (thermalReadCalibData(eeprom_buf) == false)
        {
            cliPrintf("EEPROM read failed\r\n");
            return;
        }

        if (thermalExtractParameters(eeprom_buf, &params) == false)
        {
            cliPrintf("Parameter extract failed\r\n");
            return;
        }

        if (thermalReadFrameSynced(frame_buf, &subpage) == false)
        {
            cliPrintf("Frame read failed\r\n");
            return;
        }

        ta = thermalCalculateTa(frame_buf, &params);
        reflected_temp = ta - 8.0f;

        if (thermalCalculateTemperature(frame_buf,
                                        &params,
                                        0.95f,
                                        reflected_temp,
                                        temperature_buf) == false)
        {
            cliPrintf("Temperature calculate failed\r\n");
            return;
        }

        cliPrintf("Subpage: %d\r\n", subpage);
        cliPrintf("Ta: %.2f C\r\n", ta);
        cliPrintf("First 64 temperature values:\r\n");

        for (int i = 0; i < 64; i++)
        {
            cliPrintf("[%03d] %.2f C\r\n", i, temperature_buf[i]);
        }

        cliPrintf("Temperature frame done\r\n");
    }
    else
    {
        cliPrintf("Usage:\r\n");
        cliPrintf("  th tem\r\n");
    }
}

void thermalMain(void)
{
    static uint16_t frame_buf[MLX90640_FRAME_WORD_COUNT];
    static uint16_t eeprom_buf[MLX90640_EEPROM_WORD_COUNT];
    static float temp_buf[MLX90640_PIXEL_COUNT];
    static thermal_params_t params;

    if (thermalReadCalibData(eeprom_buf) == true)
    {
        thermalExtractParameters(eeprom_buf, &params);
    }

    while (1)
    {
        if (thermalReadFrame(frame_buf) == true)
        {
            float ta = thermalCalculateTa(frame_buf, &params);

            thermalCalculateTemperature(frame_buf,
                                        &params,
                                        0.95f, // 1.0
                                        ta - 8.0f,
                                        temp_buf);

            oledDrawThermal(temp_buf);
        }

        osDelay(10);
    }
}

bool thermalSetRefreshRate(uint8_t refresh_rate)
{
    uint16_t control_reg;

    if (refresh_rate > 7)
    {
        return false;
    }

    if (thermalReadRegister(MLX90640_CONTROL_REG, &control_reg) == false)
    {
        return false;
    }

    control_reg &= ~MLX90640_REFRESH_RATE_MASK;
    control_reg |= ((uint16_t)refresh_rate << 7);

    if (thermalWriteRegister(MLX90640_CONTROL_REG, control_reg) == false)
    {
        return false;
    }

    return true;
}