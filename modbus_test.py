#!/usr/bin/env python3
"""
Script essenziale Modbus RTU per ABB ACS310
Funzioni: Test Connessione, Monitoraggio e Diagnostica Fault.
"""

from pymodbus.client import ModbusSerialClient
import time

# --- CONFIGURAZIONE ---
SERIAL_PORT = '/dev/ttyUSB0'
SLAVE_ID = 1      
BAUDRATE = 19200  
PARITY = 'N'      
STOPBITS = 1      
BYTESIZE = 8      
TIMEOUT = 2       

def get_client():
    return ModbusSerialClient(
        port=SERIAL_PORT,
        baudrate=BAUDRATE,
        bytesize=BYTESIZE,
        parity=PARITY,
        stopbits=STOPBITS,
        timeout=TIMEOUT
    )

def check_faults(client):
    """Legge la Status Word e l'ultimo codice di errore (Registro 40103)"""
    # Lettura Status Word (40051) e Fault Word 1 (40103)
    # 40051 -> address 50 | 40103 -> address 102
    try:
        # Controllo Status Word per bit di Fault (Bit 3)
        res_sw = client.read_holding_registers(address=50, count=1, device_id=SLAVE_ID)
        # Lettura ultimo codice guasto (Parametro 04.01 -> Registro 40103)
        res_err = client.read_holding_registers(address=102, count=1, device_id=SLAVE_ID)

        if not res_sw.isError() and not res_err.isError():
            sw = res_sw.registers[0]
            last_fault = res_err.registers[0]
            
            is_faulted = bool(sw & 0x0008) # Bit 3 della Status Word
            
            print("\n--- STATO DIAGNOSTICA ---")
            if is_faulted:
                print(f"STATUS: [!] IN FAULT (Codice: {last_fault})")
                # Alcuni codici comuni ABB: 1=OVERCURRENT, 2=DC OVERVOLT, 9=MOT OVERTEMP
                if last_fault == 0: print("Nota: Fault rilevato ma codice non disponibile.")
            else:
                print("STATUS: ✓ OK (Nessun Fault attivo)")
            
            if sw & 0x0080: # Bit 7: Alarm/Warning
                print("ATTENZIONE: Presente un Allarme/Warning attivo.")
        else:
            print("✗ Impossibile leggere i registri di diagnostica.")
    except Exception as e:
        print(f"✗ Errore durante il check fault: {e}")

def test_connection():
    """Test rapido parametri di marcia"""
    print("\n" + "="*40)
    print("TEST CONNESSIONE BASE")
    print("="*40)
    client = get_client()
    if not client.connect():
        print(f"✗ Errore apertura porta {SERIAL_PORT}")
        return
    
    res = client.read_holding_registers(address=0, count=4, device_id=SLAVE_ID)
    if not res.isError():
        print(f"Frequenza: {res.registers[0]/100.0} Hz")
        print(f"Corrente:  {res.registers[1]/10.0} A")
        check_faults(client)
    else:
        print(f"✗ Errore Modbus: {res}")
    client.close()

def continuous_monitoring():
    """Monitoraggio parametri e stato fault"""
    print("\n" + "="*40)
    print("MONITORAGGIO (Ctrl+C per uscire)")
    print("="*40)
    client = get_client()
    if not client.connect(): return

    try:
        while True:
            # Leggiamo freq, corrente e aggiungiamo la Status Word (addr 50)
            res_data = client.read_holding_registers(address=0, count=2, device_id=SLAVE_ID)
            res_sw = client.read_holding_registers(address=50, count=1, device_id=SLAVE_ID)
            
            if not res_data.isError() and not res_sw.isError():
                f = res_data.registers[0] / 100.0
                a = res_data.registers[1] / 10.0
                sw = res_sw.registers[0]
                
                status_str = "FAULT" if (sw & 0x0008) else "READY"
                if sw & 0x0004: status_str = "RUNNING"
                
                print(f"\rFreq: {f:6.2f}Hz | Amp: {a:5.2f}A | Stato: {status_str}   ", end='')
            time.sleep(0.5)
    except KeyboardInterrupt:
        print("\nInterrotto.")
    finally:
        client.close()

if __name__ == "__main__":
    while True:
        print("\n1. Test Connessione e Fault\n2. Monitoraggio Continuo\n0. Esci")
        scelta = input("Scelta: ")
        if scelta == "1": test_connection()
        elif scelta == "2": continuous_monitoring()
        elif scelta == "0": break
