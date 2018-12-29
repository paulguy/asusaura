#define ASUSAURA_COLORS_SIZE (15)

typedef enum {
    ASUSAURA_RAM0,
    ASUSAURA_RAM1,
    ASUSAURA_RAM2,
    ASUSAURA_RAM3,
    ASUSAURA_MAINBOARD,
    ASUSAURA_NONE
} AsusAura_ControllerType;

int asusaura_open(const char *mainbus, const char *auxbus);
void asusaura_close();
int asusaura_dev_count();
AsusAura_ControllerType asusaura_getType(unsigned int idx);
const char *asusaura_typeStr(AsusAura_ControllerType type);
int asusaura_update(unsigned int idx, const unsigned char *colors);