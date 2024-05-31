# System Architecture
Hekate system model is described in SysML using SYSMOD as a modeling methodology.

## Problem Statement
How can IoT device messages forwarded into the cloud with an affordable price and self-sufficient power supply?

## System Idea
**Hekate** is Outdoor LoRa Gateway with low power consumption based on a Raspberry Pi Pico.
The system is equipped with a low power cellular transceiver to have an internet connection via the cellular network.

Main features of **Hekate** are:
* Low power consumption
* Low Bill of Material (BOM) costs
* The Things Stacks as LoRaWAN Network Server
* Secure transmitting with state-of-the-art "LoRa Basics™ Station" protocol

## System Objectives

[Objectives](./generated/objectives.md)

## Stakeholder

![hekate_stakeholder](../papyrus/hekate/stakeholder.png)


## Base Architecture
![base_architecture](../drawio/base_architecture.drawio.png)

The hekate system consists of a **LoRa Concentrator** that is sending and receiving messages from/towards multiple **LoRa End Devices** such as Sensors. Hekate is powered by a **Battery** that could be powered by **Solar Panels**. The **Cellular Modem** connects to the **LoRaWAN Network Server**  using mobile network. The **LoRaWAN Network Server** manages the LoRaWAN Network.

### Architecture Decisions
* The LoRaWAN Network Server is provided by "The Things Network"
* The Power supply is a battery that is recharged by a solar panel.

