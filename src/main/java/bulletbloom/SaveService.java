package bulletbloom;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Properties;

/**
 * Persists and loads Java rewrite save data.
 */
public final class SaveService {
    private static final Path DEFAULT_SAVE_PATH = Path.of("save", "bullet-bloom.properties");

    private final Path savePath;

    /**
     * Creates a save service using the default local save path.
     */
    public SaveService() {
        this(DEFAULT_SAVE_PATH);
    }

    /**
     * Creates a save service using a caller-provided path.
     *
     * @param savePath path where save data should be stored
     */
    public SaveService(Path savePath) {
        this.savePath = savePath;
    }

    /**
     * Checks whether a Java save file exists.
     *
     * @return {@code true} when save data is available
     */
    public boolean exists() {
        return Files.exists(savePath);
    }

    /**
     * Saves game state to disk.
     *
     * @param data data to persist
     * @throws IOException if the save file cannot be written
     */
    public void save(SaveData data) throws IOException {
        if (savePath.getParent() != null) {
            Files.createDirectories(savePath.getParent());
        }
        Properties properties = new Properties();
        properties.setProperty("wave", Integer.toString(data.wave()));
        properties.setProperty("money", Integer.toString(data.money()));
        properties.setProperty("hearts", Integer.toString(data.hearts()));
        properties.setProperty("maxHearts", Integer.toString(data.maxHearts()));
        properties.setProperty("currentWeaponIndex", Integer.toString(data.currentWeaponIndex()));
        properties.setProperty("unlockedWeapons", encodeBooleans(data.unlockedWeapons()));
        try (OutputStream output = Files.newOutputStream(savePath)) {
            properties.store(output, "Bullet Bloom save data");
        }
    }

    /**
     * Loads game state from disk.
     *
     * @return loaded save data
     * @throws IOException if the save file cannot be read
     */
    public SaveData load() throws IOException {
        Properties properties = new Properties();
        try (InputStream input = Files.newInputStream(savePath)) {
            properties.load(input);
        }
        return new SaveData(
                parseInt(properties, "wave", 1),
                parseInt(properties, "money", 0),
                parseInt(properties, "hearts", 6),
                parseInt(properties, "maxHearts", 6),
                parseInt(properties, "currentWeaponIndex", 0),
                parseBooleans(properties.getProperty("unlockedWeapons", "true,false,false,false"), 4));
    }

    private int parseInt(Properties properties, String key, int fallback) {
        try {
            return Integer.parseInt(properties.getProperty(key, Integer.toString(fallback)));
        } catch (NumberFormatException exception) {
            return fallback;
        }
    }

    private String encodeBooleans(boolean[] values) {
        StringBuilder builder = new StringBuilder();
        for (int index = 0; index < values.length; index++) {
            if (index > 0) {
                builder.append(',');
            }
            builder.append(values[index]);
        }
        return builder.toString();
    }

    private boolean[] parseBooleans(String encoded, int minimumLength) {
        String[] parts = encoded.split(",");
        boolean[] values = new boolean[Math.max(minimumLength, parts.length)];
        for (int index = 0; index < parts.length; index++) {
            values[index] = Boolean.parseBoolean(parts[index]);
        }
        values[0] = true;
        return values;
    }
}
