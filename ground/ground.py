import pygame
import serial
import time
import serial.tools.list_ports as ports

pygame.init()
pygame.display.set_caption('Ground Station')

com_ports = list(ports.comports())
uart_ports = [] # create a list of uart ports
uart_data = [] # create a list of data from the uart ports

for i in com_ports:            
    try:
        ser = serial.Serial(i.device, 115200, timeout=1)
        uart_ports.append(ser)
        uart_data.append(ser.read(ser.in_waiting).decode("ASCII"))

        print("Connected to: ", i.device)

    except serial.SerialException:
        continue



received_packets = []


width = 1200
height = 800
screen = pygame.display.set_mode((width, height))

marker_image = pygame.image.load("marker.png")

running = True

while running:

    for i, ser in enumerate(uart_ports):
        # read data from the uart port
        try:
            data = ser.read(ser.in_waiting).decode("ASCII")
            uart_data[i] += data
        except serial.SerialException:
            continue

        if ("GND PACKET" in uart_data[i]):
            packets = uart_data[i].split("GND PACKET")

            uart_data[i] = packets[-1]

            for packet in packets[:-1]:
                
                packet = packet.strip()
                packet = packet.replace("\r", "")
                packet = packet.split("\n")

                if(len(packet) != 10):
                    continue # bad packet

                try:
                    rssi = int(packet[0][6:])
                    lqi = int(packet[1][5:])
                    
                    yaw = float(packet[2][5:])
                    pitch = float(packet[3][7:])
                    roll = float(packet[4][6:])

                    ax = float(packet[5][4:])
                    ay = float(packet[6][4:])
                    az = float(packet[7][4:])

                    channels = packet[9].split(" ")

                    for i, c in enumerate(channels):
                        channels[i] = int(c)

                except ValueError:
                    continue # bad packet

                save_data = {
                    "RSSI": rssi,
                    "LQI": lqi,
                    "Yaw": yaw,
                    "Pitch": pitch,
                    "Roll": roll,
                    "ax": ax,
                    "ay": ay,
                    "az": az,
                    "Channels": channels
                }

                received_packets.append(save_data)
        


    screen.fill((50, 50, 50))



    if len(received_packets) > 0:
        latest_packet = received_packets[-1]
            

        pygame.draw.rect(screen, (20, 20, 20), (0, 0, width/2 + 50, 250))

        font = pygame.font.Font(None, 20)

        text = font.render("RSSI: " + str(latest_packet["RSSI"]), True, (255, 255, 255))
        screen.blit(text, (10, 10))

        text = font.render("LQI: " + str(latest_packet["LQI"]), True, (255, 255, 255))
        screen.blit(text, (10, 35))

        text = font.render("Yaw: " + str(latest_packet["Yaw"]), True, (255, 255, 255))
        screen.blit(text, (10, 60))

        text = font.render("Pitch: " + str(latest_packet["Pitch"]), True, (255, 255, 255))
        screen.blit(text, (10, 85))

        text = font.render("Roll: " + str(latest_packet["Roll"]), True, (255, 255, 255))
        screen.blit(text, (10, 110))

        text = font.render("ax: " + str(latest_packet["ax"]), True, (255, 255, 255))
        screen.blit(text, (10, 135))

        text = font.render("ay: " + str(latest_packet["ay"]), True, (255, 255, 255))
        screen.blit(text, (10, 160))

        text = font.render("az: " + str(latest_packet["az"]), True, (255, 255, 255))
        screen.blit(text, (10, 185))

        text = font.render("Channels: " + str(latest_packet["Channels"]), True, (255, 255, 255))
        screen.blit(text, (10, 210))    



        # visualize the channels
        for i, c in enumerate(latest_packet["Channels"]):
            pygame.draw.rect(screen, (255, 255, 255), (width + (i - 14) * 15 - 10, 215 - c//10, 10, c // 10))
            text = font.render(str(i), True, (255, 255, 255))
            screen.blit(text, (width + (i - 14) * 15 - 10, 220))

        # draw artificial horizon

        pitch_frac = latest_packet["Pitch"] / 60

        if(abs(pitch_frac) > 1):
            pitch_frac /= abs(pitch_frac)

        pitch_frac = 0.5 + pitch_frac * 0.5

        pygame.draw.rect(screen, (30, 30, 30), (width / 2 - 205, height / 2 - 55, 410, 410))
        pygame.draw.rect(screen, (0, 125, 227), (width / 2 - 200, height / 2 - 50, 400, 400 * pitch_frac))
        pygame.draw.rect(screen, (94, 42, 0), (width / 2 - 200, height / 2 - 50 + 400 * pitch_frac, 400, 400 * (1 - pitch_frac)))


        # scale the marker
        marker = pygame.transform.scale(marker_image, (200, 16))
        # rotate the marker   
        marker = pygame.transform.rotate(marker, latest_packet["Roll"])
        marker_rect = marker.get_rect()
        screen.blit(marker, (width / 2 - marker_rect.width/2, height / 2 + 200 - 50 - marker_rect.height/2))

        # render G-force
        g_force = (latest_packet["ax"]**2 + latest_packet["ay"]**2 + latest_packet["az"]**2)**0.5
        g_force = round(g_force / 10, 1)

        text = font.render(str(g_force) + " g", True, (245, 230, 66))
        screen.blit(text, (width / 2 - 190, height / 2 - 70 + 400))

        
        # turn coordinator
        pygame.draw.rect(screen, (50, 50, 50), (width / 2 - 50, height / 2 - 40, 100, 30), border_radius=15)
        ball_offset = latest_packet["ay"] * 10
        if(abs(ball_offset) > 40):
            ball_offset = 40 * ball_offset / abs(ball_offset)
        pygame.draw.circle(screen, (20, 20, 20), (width / 2 + ball_offset, height / 2 - 25), 10)

    pygame.display.flip()

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False


pygame.quit()