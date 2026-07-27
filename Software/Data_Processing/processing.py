import pandas as pd
from sklearn.decomposition import IncrementalPCA
from sklearn.preprocessing import StandardScaler
from scipy.signal import butter, filtfilt

# --- CONFIGURATION ---
INPUT_CSV = 'result.csv'            
OUTPUT_CSV = 'gyro_processed.csv'

# --- RESPIRATION BANDPASS FILTER CONFIGURATION ---
FS = 100.0       # Sample rate (Hz) - 100Hz from ESP32
LOWCUT = 0.1     # Lowest breathing frequency (6 breaths/min)
HIGHCUT = 0.5    # Highest breathing frequency (30 breaths/min)
ORDER = 2        # Low order to prevent ringing/distortion on slow waves

def butter_bandpass_filter(data, lowcut, highcut, fs, order=2):
    nyq = 0.5 * fs
    low = lowcut / nyq
    high = highcut / nyq
    # Create a bandpass filter
    b, a = butter(order, [low, high], btype='band', analog=False)
    # filtfilt applies it forwards and backwards for zero phase delay (keeps it synced with your pulse!)
    y = filtfilt(b, a, data, axis=0)
    return y

def main():
    print(f"Loading data from {INPUT_CSV}...")
    
    try:
        df = pd.read_csv(INPUT_CSV, low_memory=False)
    except FileNotFoundError:
        print(f"Error: Could not find {INPUT_CSV}.")
        return

    print("Cleaning data...")

    # 1. SWAPPED TO GYROSCOPE COLUMNS
    # We are now looking purely at the rotational velocity of the chest
    sensor_columns = [
        'S1_Gx', 'S1_Gy', 'S1_Gz', 
        'S2_Gx', 'S2_Gy', 'S2_Gz', 
        'S3_Gx', 'S3_Gy', 'S3_Gz'
    ]
    
    for col in sensor_columns:
        df[col] = pd.to_numeric(df[col], errors='coerce')

    df = df.dropna(subset=sensor_columns).reset_index(drop=True)

    s1_data = df[['S1_Gx', 'S1_Gy', 'S1_Gz']].values
    s2_data = df[['S2_Gx', 'S2_Gy', 'S2_Gz']].values
    s3_data = df[['S3_Gx', 'S3_Gy', 'S3_Gz']].values

    print(f"Applying {LOWCUT}Hz - {HIGHCUT}Hz Bandpass Filter for Respiration...")

    # 2. APPLY BANDPASS FILTER
    s1_filtered = butter_bandpass_filter(s1_data, LOWCUT, HIGHCUT, FS, ORDER)
    s2_filtered = butter_bandpass_filter(s2_data, LOWCUT, HIGHCUT, FS, ORDER)
    s3_filtered = butter_bandpass_filter(s3_data, LOWCUT, HIGHCUT, FS, ORDER)

    print("Scaling data and applying IPCA to find the dominant rotational axis...")

    scaler1 = StandardScaler()
    scaler2 = StandardScaler()
    scaler3 = StandardScaler()

    s1_scaled = scaler1.fit_transform(s1_filtered)
    s2_scaled = scaler2.fit_transform(s2_filtered)
    s3_scaled = scaler3.fit_transform(s3_filtered)

    ipca = IncrementalPCA(n_components=1, batch_size=100)

    s1_ipca = ipca.fit_transform(s1_scaled)
    s2_ipca = ipca.fit_transform(s2_scaled)
    s3_ipca = ipca.fit_transform(s3_scaled)

    # 3. BUILD OUTPUT
    output_df = pd.DataFrame({
        'ESP_Time_ms': df['ESP_Time_ms'],
        'S1_IPCA': s1_ipca.flatten(),
        'S2_IPCA': s2_ipca.flatten(),
        'S3_IPCA': s3_ipca.flatten(),
        'Sync_Pulse': df['Sync_Pulse']
    })

    output_df.to_csv(OUTPUT_CSV, index=False)
    print(f"Success! Processed breathing data saved to '{OUTPUT_CSV}'.")

if __name__ == "__main__":
    main()