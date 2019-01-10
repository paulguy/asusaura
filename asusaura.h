typedef enum {
    ASUSAURA_RAM0 = 0,
    ASUSAURA_RAM1,
    ASUSAURA_RAM2,
    ASUSAURA_RAM3,
    ASUSAURA_MAINBOARD,
    ASUSAURA_NONE
} AsusAura_ControllerType;

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} AsusAura_Color;

int asusaura_open(const char *mainbus, const char *auxbus);
void asusaura_close();
int asusaura_dev_count();
AsusAura_ControllerType asusaura_getType(unsigned int idx);
const char *asusaura_typeStr(AsusAura_ControllerType type);
unsigned int asusaura_getColorReq(unsigned int idx, AsusAura_Color **c);
void asusaura_freeColorReq(AsusAura_Color *c);
int asusaura_update(unsigned int idx, const AsusAura_Color *c);
