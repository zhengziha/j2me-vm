import javax.microedition.media.Manager;
import javax.microedition.media.Player;
import javax.microedition.media.MediaException;

public class AudioTest {
    public static void main(String[] args) {
        System.out.println("AudioTest started");
        
        try {
            // 创建一个音频播放器
            System.out.println("Creating audio player...");
            Player player = Manager.createPlayer("audio/sample");
            
            if (player != null) {
                System.out.println("Player created successfully");
                
                // 实现播放器
                System.out.println("Realizing player...");
                player.realize();
                
                // 预取播放器
                System.out.println("Prefetching player...");
                player.prefetch();
                
                // 播放声音
                System.out.println("Starting playback...");
                player.start();
                
                // 等待一段时间让声音播放完成
                System.out.println("Playing sound... (waiting 3 seconds)");
                Thread.sleep(3000);
                
                // 停止播放
                System.out.println("Stopping playback...");
                player.stop();
                
                // 关闭播放器
                System.out.println("Closing player...");
                player.close();
                
                System.out.println("AudioTest completed successfully");
            } else {
                System.out.println("Failed to create player");
            }
            
        } catch (MediaException e) {
            System.out.println("MediaException: " + e.getMessage());
            e.printStackTrace();
        } catch (InterruptedException e) {
            System.out.println("InterruptedException: " + e.getMessage());
            e.printStackTrace();
        } catch (Exception e) {
            System.out.println("Exception: " + e.getMessage());
            e.printStackTrace();
        }
    }
}
