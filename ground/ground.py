import pygame
import serial
import time
import serial.tools.list_ports as ports
from datetime import datetime

import numpy as np

import pandas as pd

now = datetime.now()
dt_string = now.strftime("%Y_%m_%d_%H:%M")
file_name = "flights/" + dt_string + ".csv"
print("Saving data to: ", file_name)

columns=["RSSI", "LQI", "Time", "Yaw", "Pitch", "Roll", "ax", "ay", "az"]

for i in range(14):
    columns.append("Channel_" + str(i))

columns.append("Mode")

saved_packets = pd.DataFrame()

recording = False

def main():

    global saved_packets
    global file_name
    global recording

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

    last_packet_time = 0

    running = True

    record_button = pygame.Rect(width//2 + 60, 15, 50, 50)

    while running:

        for i, ser in enumerate(uart_ports):
            # read data from the uart port
            try:
                data = ser.read(ser.in_waiting).decode("ASCII")
                uart_data[i] += data
            except:
                continue

            if ("GND PACKET" in uart_data[i]):
                packets = uart_data[i].split("GND PACKET")

                uart_data[i] = packets[-1]

                for packet in packets[:-1]:
                    
                    packet = packet.strip()
                    packet = packet.replace("\r", "")
                    packet = packet.split("\n")

                    if(len(packet) != 12):
                        continue # bad packet

                    try:
                        rssi = int(packet[0][6:])
                        lqi = int(packet[1][5:])
                        
                        t = int(packet[2][6:])

                        yaw = float(packet[3][5:])
                        pitch = float(packet[4][7:])
                        roll = float(packet[5][6:])

                        ax = float(packet[6][4:])
                        ay = float(packet[7][4:])
                        az = float(packet[8][4:])

                        packet[10] = packet[10].strip()
                        channels = packet[10].split(" ")

                        for i, c in enumerate(channels):
                            channels[i] = int(c)

                        mode = int(packet[11][6:])

                        last_packet_time = time.time()

                    except ValueError as e:
                        # print(e)
                        continue # bad packet

                    save_data = {
                        "RSSI": rssi,
                        "LQI": lqi,
                        "Time": t,
                        "Yaw": yaw,
                        "Pitch": pitch,
                        "Roll": roll,
                        "ax": ax,
                        "ay": ay,
                        "az": az,
                        "Channels": channels,
                        "Mode": mode
                    }

                    received_packets.append(save_data)

                    if recording:
                        data = [rssi, lqi, t, yaw, pitch, roll, ax, ay, az]
                        data += channels
                        data += [mode]

                        saved_packets = saved_packets._append(pd.DataFrame([data], columns=columns), ignore_index=True)

        screen.fill((50, 50, 50))



        if len(received_packets) > 0:
            latest_packet = received_packets[-1]
                

            pygame.draw.rect(screen, (20, 20, 20), (0, 0, width/2 + 50, 285))

            font = pygame.font.Font(None, 20)
            font_middle = pygame.font.Font(None, 30)
            font_big = pygame.font.Font(None, 40)


            text = font.render("RSSI: " + str(latest_packet["RSSI"]), True, (255, 255, 255))
            screen.blit(text, (10, 10))

            text = font.render("LQI: " + str(latest_packet["LQI"]), True, (255, 255, 255))
            screen.blit(text, (10, 35))

            text = font.render("Time: " + str(latest_packet["Time"]), True, (255, 255, 255))
            screen.blit(text, (10, 60))

            text = font.render("Yaw: " + str(latest_packet["Yaw"]), True, (255, 255, 255))
            screen.blit(text, (10, 85))

            text = font.render("Pitch: " + str(latest_packet["Pitch"]), True, (255, 255, 255))
            screen.blit(text, (10, 110))

            text = font.render("Roll: " + str(latest_packet["Roll"]), True, (255, 255, 255))
            screen.blit(text, (10, 135))

            text = font.render("ax: " + str(latest_packet["ax"]), True, (255, 255, 255))
            screen.blit(text, (10, 160))

            text = font.render("ay: " + str(latest_packet["ay"]), True, (255, 255, 255))
            screen.blit(text, (10, 185))

            text = font.render("az: " + str(latest_packet["az"]), True, (255, 255, 255))
            screen.blit(text, (10, 210))

            text = font.render("Channels: " + str(latest_packet["Channels"]), True, (255, 255, 255))
            screen.blit(text, (10, 235))

            text = font.render("Mode: " + str(latest_packet["Mode"]), True, (255, 255, 255))
            screen.blit(text, (10, 260))



            text = font_big.render("Elapsed: " + str(round(latest_packet["Time"] / 1000, 1)) + " s", True, (255, 255, 255))
            screen.blit(text, (10, 295))

            # visualize the channels
            for i, c in enumerate(latest_packet["Channels"]):
                pygame.draw.rect(screen, (255, 255, 255), (width + (i - 14) * 15 - 10, 265 - c//10, 10, c // 10))
                text = font.render(str(i), True, (255, 255, 255))
                screen.blit(text, (width + (i - 14) * 15 - 10, 270))

            # flight mode
            if latest_packet["Mode"] == 0:
                text = font_middle.render("Manual mode", True, (255, 255, 255))
            elif latest_packet["Mode"] == 1:
                text = font_middle.render("Take-off mode", True, (17, 91, 212))
            elif latest_packet["Mode"] == 2:
                text = font_middle.render("Fly-by-wire mode", True, (8, 138, 47))
            elif latest_packet["Mode"] == 255:
                text = font_middle.render("Recovery mode", True, (140, 0, 14))

            # get the width of the text
            text_rect = text.get_rect()
            screen.blit(text, (width - 115 - text_rect.width / 2, 15))

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

        if(time.time() - last_packet_time > 1):
            # render a red circle
            pygame.draw.circle(screen, (255, 0, 0), (width/2, 40), 20)
        else:
            # render a green circle
            pygame.draw.circle(screen, (0, 255, 0), (width/2, 40), 20)

        if recording:
            pygame.draw.rect(screen, (20, 20, 20), record_button)
            # draww two vertical lines
            pygame.draw.line(screen, (255, 255, 255), (width//2 + 75, 25), (width//2 + 75, 55), 5)
            pygame.draw.line(screen, (255, 255, 255), (width//2 + 95, 25), (width//2 + 95, 55), 5)
            # add small text under the button
            font = pygame.font.Font(None, 18)
            text = font.render("Recording", True, (255, 255, 255))
            screen.blit(text, (width//2 + 55, 75))
        else:
            pygame.draw.rect(screen, (40, 40, 40), record_button)
            # draw a triangle
            pygame.draw.polygon(screen, (255, 255, 255), [(width//2 + 75, 30), (width//2 + 75, 50), (width//2 + 95, 40)])

        pygame.display.flip()

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            if event.type == pygame.MOUSEBUTTONDOWN:
                if record_button.collidepoint(event.pos):
                    recording = not recording
            # check if space is pressed
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_SPACE:
                    recording = not recording

    pygame.quit()

    if len(saved_packets) > 0:
        saved_packets.to_csv(file_name, index=False)

try:
    main()
except KeyboardInterrupt:
    if len(saved_packets) > 0:
        saved_packets.to_csv(file_name, index=False)