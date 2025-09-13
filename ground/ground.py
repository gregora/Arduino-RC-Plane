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

columns=["RSSI", "LQI", "Time", "Yaw", "Pitch", "Roll", "q1", "q2", "q3", "q4", "ax", "ay", "az", "Latitude", "Longitude", "Altitude", "Satellites"]

for i in range(7):
    columns.append("Channel_" + str(i))

columns.append("Mode")

saved_packets = pd.DataFrame()

recording = False

hide_location = False   # dont show the location on UI

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
    screen = pygame.display.set_mode((width, height), pygame.RESIZABLE)

    marker_image = pygame.image.load("marker.png")

    last_packet_time = 0

    running = True


    while running:

        record_button = pygame.Rect(width//2 + 60, 15, 50, 50)


        for i, ser in enumerate(uart_ports):
            # read data from the uart port
            try:
                data = ser.read(ser.in_waiting).decode("ASCII")
                uart_data[i] += data
            except:
                continue

            if ("GND PACKET" in uart_data[i]):
                packets = uart_data[i].split("GND PACKET")

                uart_data[i] = "GND PACKET" + packets[-1]

                for packet in packets[:-1]:
                    
                    packet = packet.strip()
                    packet = packet.replace("\r", "")
                    packet = packet.split("\n")


                    if(len(packet) != 20):
                        continue # bad packet

                    try:
                        rssi = int(packet[0][6:])
                        lqi = int(packet[1][5:])
                        
                        t = int(packet[2][6:])

                        yaw = float(packet[3][5:])
                        pitch = float(packet[4][7:])
                        roll = float(packet[5][6:])

                        q1 = float(packet[6][4:])
                        q2 = float(packet[7][4:])
                        q3 = float(packet[8][4:])
                        q4 = float(packet[9][4:])

                        ax = float(packet[10][4:])
                        ay = float(packet[11][4:])
                        az = float(packet[12][4:])

                        latitude = float(packet[13][9:])
                        longitude = float(packet[14][10:])
                        altitude = float(packet[15][9:])
                        satellites = int(packet[16][12:])
                        
                        packet[18] = packet[18].strip()
                        channels = packet[18].split(" ")

                        for i, c in enumerate(channels):
                            channels[i] = int(c)

                        mode = int(packet[19][6:])

                        last_packet_time = time.time()

                    except ValueError as e:
                        print(e)
                        continue # bad packet

                    save_data = {
                        "RSSI": rssi,
                        "LQI": lqi,
                        "Time": t,
                        "Yaw": yaw,
                        "Pitch": pitch,
                        "Roll": roll,
                        "q1": q1,
                        "q2": q2,
                        "q3": q3,
                        "q4": q4,
                        "ax": ax,
                        "ay": ay,
                        "az": az,
                        "Latitude": latitude,
                        "Longitude": longitude,
                        "Altitude": altitude,
                        "Satellites": satellites,
                        "Channels": channels,
                        "Mode": mode
                    }

                    received_packets.append(save_data)

                    if recording:
                        data = [rssi, lqi, t, yaw, pitch, roll, q1, q2, q3, q4, ax, ay, az, latitude, longitude, altitude, satellites]
                        data += channels
                        data += [mode]

                        saved_packets = saved_packets._append(pd.DataFrame([data], columns=columns), ignore_index=True)

        screen.fill((50, 50, 50))



        if len(received_packets) > 0:
            latest_packet = received_packets[-1]
                

            pygame.draw.rect(screen, (20, 20, 20), (0, 0, width/2 + 50, 275))

            font = pygame.font.Font(None, 20)
            font_middle = pygame.font.Font(None, 30)
            font_big = pygame.font.Font(None, 40)

            # left side

            text = font.render("RSSI: " + str(latest_packet["RSSI"]), True, (255, 255, 255))
            screen.blit(text, (10, 10))

            text = font.render("LQI: " + str(latest_packet["LQI"]), True, (255, 255, 255))
            screen.blit(text, (10, 35))


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
            screen.blit(text, (10, 240))



            # right side

            text = font.render("Time: " + str(latest_packet["Time"]), True, (255, 255, 255))
            screen.blit(text, (200, 10))

            text = font.render("Mode: " + str(latest_packet["Mode"]), True, (255, 255, 255))
            screen.blit(text, (200, 35))

            if not hide_location:

                text = font.render("Latitude: " + str(latest_packet["Latitude"]), True, (255, 255, 255))
                screen.blit(text, (200, 85))

                text = font.render("Longitude: " + str(latest_packet["Longitude"]), True, (255, 255, 255))
                screen.blit(text, (200, 110))

            text = font.render("Altitude: " + str(latest_packet["Altitude"]), True, (255, 255, 255))
            screen.blit(text, (200, 135))

            text = font.render("Satellites: " + str(latest_packet["Satellites"]), True, (255, 255, 255))
            screen.blit(text, (200, 160))



            text = font_big.render("Elapsed: " + str(round(latest_packet["Time"] / 1000, 1)) + " s", True, (255, 255, 255))
            screen.blit(text, (10, 285))

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
            elif latest_packet["Mode"] == 3:
                text = font_middle.render("Automatic mode", True, (255, 165, 0))
            elif latest_packet["Mode"] == 255:
                text = font_middle.render("Recovery mode", True, (140, 0, 14))

            # get the width of the text
            text_rect = text.get_rect()
            screen.blit(text, (width - 115 - text_rect.width / 2, 15))




            ### Artificial horizon ###

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
            marker = pygame.transform.rotate(marker, - latest_packet["Roll"])
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



            ### Minimap ###

            # render position
            pygame.draw.rect(screen, (20, 20, 20), (10, height / 2 - 30, 370, 370))

            line = []

            # get average position
            if len(received_packets) > 1:
                avg_pos = np.array([0.0, 0.0])
                for p in received_packets[-10000::5]:
                    # check if data is valid
                    if p["Latitude"] <= 0.1 and p["Longitude"] <= 0.1:
                        continue

                    avg_pos += np.array([p["Latitude"], p["Longitude"]])
                    line.append([p["Longitude"], p["Latitude"]])
                avg_pos /= len(received_packets[-10000::5])
                #print(avg_pos)

                map_scale_lat = 40075 / 360  # full width at 1 km
                map_scale_long = 40075 / 360 * np.cos(avg_pos[0] * 3.1415 / 180) # full height at 1 km

                for l in line:
                    l[0] = int(10 + 370/2 + 370 * (l[0] - avg_pos[1]) * map_scale_long)
                    l[1] = int(height / 2 - 30 + 370/2 - 370 * (l[1] - avg_pos[0]) * map_scale_lat)

                if len(line) > 2:
                    pygame.draw.lines(screen, (255, 255, 255), False, line, 1)

                # render marker
                pygame.draw.circle(screen, (255, 0, 0), (int(10 + 370/2 + 370 * (latest_packet["Longitude"] - avg_pos[1]) * map_scale_long), int(height / 2 - 30 + 370/2 - 370 * (latest_packet["Latitude"] - avg_pos[0]) * map_scale_lat)), 3)
                




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

        width, height = screen.get_size()

    pygame.quit()

    if len(saved_packets) > 0:
        try:
            saved_packets.to_csv(file_name, index=False)
        except:
            saved_packets.to_csv(file_name.replace("flights/", ""), index=False)

try:
    main()
except KeyboardInterrupt:
    if len(saved_packets) > 0:
        try:
            saved_packets.to_csv(file_name, index=False)
        except:
            saved_packets.to_csv(file_name.replace("flights/", ""), index=False)
