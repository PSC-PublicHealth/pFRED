package edu.pitt.phdl.frednavigator.topic1;

import java.net.URL;
import java.util.ResourceBundle;
import java.util.Scanner;
import javafx.beans.value.ChangeListener;
import javafx.beans.value.ObservableValue;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.Group;
import javafx.scene.Scene;
import javafx.scene.chart.LineChart;
import javafx.scene.chart.NumberAxis;
import javafx.scene.chart.XYChart;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.Slider;
import javafx.scene.control.Tab;
import javafx.scene.control.TabPane;
import javafx.scene.layout.BorderPane;
import javafx.scene.media.Media;
import javafx.scene.media.MediaPlayer;
import javafx.stage.Stage;

/**
 *
 * @author jackpaparian
 */
public class Topic1Controller implements Initializable {

  String r0Value = "2.5";
  LineChart chart;
  String fileVariable;
  BorderPane borderPane;
  MediaPlayer mediaPlayer;
  //Slider r0Slider;
  @FXML
  private TabPane tabPaneTopic1;
  @FXML
  private Tab tabSEIR;
  @FXML
  private BorderPane brdrPaneSEIR;
  @FXML
  private Tab tabIncidence;
  @FXML
  private Slider r0Slider;

  @FXML
  private void closeApplication(ActionEvent event)
  {
    System.exit(0);
  }

  @FXML
  private void newSTab(ActionEvent event)
  {
    fileVariable = "S";
    makeNewTab(fileVariable);
  }

  @FXML
  private void newETab(ActionEvent event)
  {
    fileVariable = "E";
    makeNewTab(fileVariable);
  }

  @FXML
  private void newITab(ActionEvent event)
  {
    fileVariable = "I";
    makeNewTab(fileVariable);
  }

  @FXML
  private void newRTab(ActionEvent event)
  {
    fileVariable = "R";
    makeNewTab(fileVariable);
  }

  @FXML
  private void newCTab(ActionEvent event)
  {
    fileVariable = "C";
    makeNewTab(fileVariable);
  }

  private void makeNewTab(String variable)
  {
    Tab newTab = new Tab();
    newTab.setText(variable);

    borderPane = new BorderPane();

//    r0Slider = makeR0Slider();
 
    /*r0Slider.setOnMouseDragReleased(new EventHandler<MouseDragEvent>() {
     public void handle(MouseDragEvent me) {
     System.out.println("Mouse Drag Event");
     //Number old_val, Number new_val) {
     double new_val = r0Slider.getValue();
     String new_val_string = (new_val * 10) + "";
     //Test if slider has moved to the next slider tick
     if (new_val_string.charAt(3) == '0') {
     r0Value = new_val_string.charAt(0) + "." + 
     new_val_string.charAt(1);
     if (r0Value.charAt(0) > '1' && r0Value.charAt(2) == '0') {
     r0Value = r0Value.charAt(0) + "";
     }
     chart = makeLineChart(fileVariable, r0Value);
     borderPane.setCenter(chart);
     }
     }
     });*/

    borderPane.setTop(r0Slider);
    chart = makeLineChart(variable, r0Value);
    borderPane.setCenter(chart);

    Button movieButton = new Button();
    movieButton.setOnAction(new EventHandler<ActionEvent>() {
      @Override
      public void handle(ActionEvent e)
      {
        openMovie();
      }
    });
    movieButton.setText("Play Movie");
    borderPane.setLeft(movieButton);


    newTab.setContent(borderPane);
    //tabPane.getTabs().add(newTab);
  }

  /**
   *
   */
  public void play()
  {
    MediaPlayer.Status status = mediaPlayer.getStatus();
    if (status == MediaPlayer.Status.UNKNOWN
            || status == MediaPlayer.Status.HALTED)
    {
      //System.out.println("Player is in a bad or unknown state, can't play.");
      return;
    }

    if (status == MediaPlayer.Status.PAUSED
            || status == MediaPlayer.Status.STOPPED
            || status == MediaPlayer.Status.READY)
    {
      mediaPlayer.play();
    }
  }

  /**
   *
   */
  public void openMovie()
  {
    String MEDIA_URL = "file:///Users/jackpaparian/Desktop/fred_gaia_ts_R0=1.4.1.mp4";
    mediaPlayer = new MediaPlayer(new Media(MEDIA_URL));
    mediaPlayer.setAutoPlay(true);
    MediaControl mediaControl = new MediaControl(mediaPlayer);
    mediaControl.setMinSize(720, 420);
    mediaControl.setPrefSize(720, 420);
    mediaControl.setMaxSize(720, 420);
    Group group = new Group();
    Stage stage = new Stage();
    Scene scene = new Scene(group);
    stage.setScene(scene);
    group.getChildren().add(mediaControl);
    stage.show();
    play();
  }

  private LineChart makeLineChart(String variable, String r0Value)
  {
    NumberAxis xAxis = new NumberAxis();
    NumberAxis yAxis = new NumberAxis();
    xAxis.setLabel("Days");


    //creating the chart
    LineChart<Number, Number> lineChart =
            new LineChart<>(xAxis, yAxis);

    lineChart.setTitle("R0=" + r0Value + " " + variable);
    //defining a series
    XYChart.Series series = new XYChart.Series();
    series.setName("Number of People");

    String pathToExperimentDirectory =
            FREDNavigatorTopic1.pathToFRED + "RESULTS/JOB/"
            + FREDNavigatorTopic1.keyToJob.get("R0=" + r0Value)
            + "/DATA/REPORTS/" + variable + "_daily-0.dat";

    Scanner scanner =
            FREDNavigatorTopic1.getFileScanner(pathToExperimentDirectory);
    for (int i = 0; i < 7; i++)
    {
      scanner.next();
    }

    while (scanner.hasNext())
    {
      double x = scanner.nextDouble();
      double y = scanner.nextDouble();
      series.getData().add(new XYChart.Data(x, y));

      //These variables are not used. It is needed so the
      //scanner can skip ahead
      for (int i = 0; i < 6; i++)
      {
        scanner.next();
      }
    }

    lineChart.getData().add(series);
    lineChart.setCreateSymbols(false);
    return lineChart;
  }

  /**
   *
   * @param url
   * @param rb
   */
  @Override
  public void initialize(URL url, ResourceBundle rb)
  {
    this.r0Slider.valueProperty().addListener(new ChangeListener<Number>() {
      @Override
      public void changed(ObservableValue<? extends Number> ov,
              Number old_val, Number new_val)
      {
        double new_val_double = new_val.doubleValue();
        String new_val_string = (new_val_double * 10) + "";
        //Test if slider has moved to the next slider tick
        if (new_val_string.charAt(3) == '0')
        {
          r0Value = new_val_string.charAt(0) + "."
                  + new_val_string.charAt(1);
          if (r0Value.charAt(0) > '1' && r0Value.charAt(2) == '0')
          {
            r0Value = r0Value.charAt(0) + "";
          }
          chart = makeLineChart("I", r0Value);
          brdrPaneSEIR.setCenter(chart);
        }
//        System.out.println("test: " + old_val + ", " + new_val);
//        brdrPaneSEIR.setCenter(new Label("test: " + old_val + ", " + new_val));
      }
    });
  }
}
