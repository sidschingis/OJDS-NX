/*
 * Open Joystick Display Server NX
 * Copyright (C) 2019 Nichole Mattera
 * This file is part of OJDS-NX <https://github.com/NicholeMattera/OJDS-NX>.
 *
 * OJDS-NX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OJDS-NX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OJDS-NX. If not, see <http://www.gnu.org/licenses/>.
 */

#include <arpa/inet.h>
#include <limits.h>
#include <sys/socket.h>

#include "server.hpp"

using namespace std;

int setupServerSocket()
{
    auto server_sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(56709);

    auto addr = (struct sockaddr *)&server;
    while (bind(server_sock, addr, sizeof(server)) < 0)
    {
        svcSleepThread(1e+9L);
    }

    listen(server_sock, 3);

    return server_sock;
}

class Axes
{
public:
    float lx;
    float ly;
    float rx;
    float ry;

    string toJson()
    {
        string json = string("[") + std::to_string(lx) + "," + std::to_string(ly) + "," + std::to_string(rx) + "," + std::to_string(ry) + "]";

        return json;
    }
};
class Button
{
public:
    bool pressed;
    int value;

    string toJson()
    {
        string json = string("{") +
                      string("\"pressed\" : ") + std::to_string(pressed) + string(",") +
                      string("\"value\"  : ") + std::to_string(value) +
                      string("}");

        return json;
    }
};

class Buttons
{
public:
    string buttonJson = string("");

    void addButton(Button button, int i)
    {
        if (i != 0)
        {
            buttonJson = buttonJson + string(",");
        }

        buttonJson = buttonJson + button.toJson();
    }

    string toJson()
    {
        string json = string("[") + buttonJson + string("]");

        return json;
    }
};

class JsonPayload
{
public:
    Axes axes;
    Buttons buttons;

    string toJson()
    {
        string json = string("{") +
                      string("\"axes\" : ") + axes.toJson() + string(",") +
                      string("\"buttons\" : ") + buttons.toJson() + string(",") +
                      string("\"connected\" : true,") +
                      string("\"id\" : \"Nintendo Switch\",") +
                      string("\"index\" : 0,") +
                      string("\"mapping\" : \"standard\",") +
                      string("\"timestamp\" : 0.0") +
                      string("}");

        return json;
    }
};

string buildJSONPayload(u64 keys, JoystickPosition lPos, JoystickPosition rPos)
{
    JsonPayload payload;

    Axes axes;
    axes.lx = (double)lPos.dx / (double)SHRT_MAX;
    axes.ly = (double)lPos.dy * -1 / (double)SHRT_MAX;
    axes.rx = (double)rPos.dx / (double)SHRT_MAX;
    axes.ry = (double)rPos.dy * -1 / (double)SHRT_MAX;
    payload.axes = axes;

    Buttons btns;

    for (int i = 0; i < 16; i++)
    {
        bool keyPressed = keys & BIT(i);
        Button button;
        button.pressed = keyPressed;
        button.value = keyPressed ? 1 : 0;
        btns.addButton(button, i);
    }

    payload.buttons = btns;

    auto result = payload.toJson();
    result.insert(0, to_string(result.size()) + "#");

    return result;
}
