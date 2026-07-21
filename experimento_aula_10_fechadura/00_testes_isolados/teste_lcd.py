from components import LCD1602


def main():
    lcd = LCD1602()

    try:
        lcd.show("Fechadura", "LCD: OK")
        print(f"LCD detectado em 0x{lcd.address:02X}")
    finally:
        lcd.close()


if __name__ == "__main__":
    main()
