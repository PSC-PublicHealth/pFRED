package edu.pitt.phdl.frednavigator.topic1;

import edu.pitt.phdl.frednavigator.FredNavigator;
import java.io.File;
import java.util.HashMap;
import java.util.Scanner;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.stage.Stage;

/**
 *
 * @author jackpaparian
 */
public class FREDNavigatorTopic1 extends Application {

    public static HashMap<String, String> keyToJob = mapKeyToJob();
     
    @Override
    public void start(Stage stage) throws Exception 
    {
        Parent root = FXMLLoader.load(getClass().getResource("topic1.fxml"));
        Scene scene = new Scene(root);
        stage.setScene(scene);
        stage.show();
    }

    private static HashMap<String, String> mapKeyToJob() 
    {
      
      Scanner keyScanner = FredNavigator.getFileScanner(FredNavigator.pathToFRED + "RESULTS/KEY");
        
      FREDNavigatorTopic1.keyToJob = new HashMap<>();

      while(keyScanner.hasNext())
      {
        String key = keyScanner.next();
        String ID = keyScanner.next();
        FREDNavigatorTopic1.keyToJob.put(key, ID);
      }
      return FREDNavigatorTopic1.keyToJob;
    }

    
    /**
     * The main() method is ignored in correctly deployed JavaFX application.
     * main() serves only as fallback in case the application can not be
     * launched through deployment artifacts, e.g., in IDEs with limited FX
     * support. NetBeans ignores main().
     *
     * @param args the command line arguments
     */
    public static void main(String[] args) 
    {
        launch(args);
    }
}
