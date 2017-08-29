/*
 * Copyright (C) 2017 Canonical
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
DefinitionBlock ("", "DSDT", 2, "UBUNTU", "_DSDT_01", 0x00000001)
{
	Scope (_SB) {
		/* Customized Hotkey Device */
		Device (HOTK) {
			Name (DRDY, 0)	/* Driver ready flag */
			Name (EKEY, 0)	/* Event code */
			Name (_HID, "UBT0002")

			Method (_STA) { 
				Return (0xF)
			}

			/* Driver Load = 1 & unload = 0 */
			Method (ENAB, 1) {
				Store (Arg0, DRDY)
			}

			/* Report event code to driver */
			Method (GETK) {
				Return (EKEY)
			}

			/* Notify function - non-reentrant */
			Method (NTFY) {
				if (DRDY) {
					Notify (HOTK, 0x80)
				}
			}
		}

		/* Embedded Controller Device */
		Device (EC) {
			Name (_HID, EisaId ("PNP0C09"))
			Method (_STA) { 
				return (0xF)
			}
			
			/* Note: event number 0x80~0x83 can be read from EC RAM */

			/* Volume up */
			Method (_Q01) {
				Store(0x80, \_SB.HOTK.EKEY)
				\_SB.HOTK.NTFY()
			}

			/* Volume down */
			Method (_Q02) { 
				Store(0x81, \_SB.HOTK.EKEY)
				\_SB.HOTK.NTFY()
			}

			/* Radio toggle */
			Method (_Q03) { 
				Store(0x82, \_SB.HOTK.EKEY)
				\_SB.HOTK.NTFY()
			}
		}
	}
}

