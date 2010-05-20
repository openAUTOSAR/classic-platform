

#include "Dio.h"
#include "stm32f10x_gpio.h"

typedef GPIO_TypeDef* GPIO_TypeDefPtr;
const GPIO_TypeDefPtr GPIO_ports[] = { GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF };

Dio_LevelType Dio_ReadChannel(Dio_ChannelType channelId) {
  int pin = channelId % 16;

  uint16_t port_index = channelId / 16;

  return (Dio_LevelType) GPIO_ReadInputDataBit(GPIO_ports[port_index], 1<<pin);
}


void Dio_WriteChannel(Dio_ChannelType channelId, Dio_LevelType level) {
  int pin = channelId % 16;

  uint16_t port_index = channelId / 16;

  GPIO_WriteBit(GPIO_ports[port_index], (1 << pin), level);
}

void Dio_WritePort(Dio_PortType portId, Dio_PortLevelType level) {
  GPIO_Write(GPIO_ports[portId], level);
}

/*
 * Unimplemented:
 *
Dio_PortLevelType Dio_ReadPort(Dio_PortType portId);
Dio_PortLevelType Dio_ReadChannelGroup( const Dio_ChannelGroupType *channelGroupIdPtr );
void Dio_WriteChannelGroup(const Dio_ChannelGroupType *channelGroupIdPtr, Dio_PortLevelType level);
*/
