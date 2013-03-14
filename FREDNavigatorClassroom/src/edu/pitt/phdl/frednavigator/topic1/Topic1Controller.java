package edu.pitt.phdl.frednavigator.topic1;

import edu.pitt.phdl.frednavigator.FredNavigatorContext;
import edu.pitt.phdl.frednavigator.HomepageController;
import java.io.IOException;
import java.net.URL;
import java.util.ResourceBundle;
import java.util.Scanner;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.beans.value.ChangeListener;
import javafx.beans.value.ObservableValue;
import javafx.collections.ObservableList;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.fxml.Initializable;
import javafx.scene.Group;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.chart.LineChart;
import javafx.scene.chart.NumberAxis;
import javafx.scene.chart.XYChart;
import javafx.scene.control.Button;
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

  String r0Value = "1.5";
  String fileVariable;

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
  private BorderPane brdrPaneIncidence;
  @FXML
  private Tab tabAttackRate;
  @FXML
  private BorderPane brdrPaneAttackRate;
  @FXML 
  private Tab tabPrevalence;
  @FXML
  private BorderPane brdrPanePrevalence;
  @FXML
  private Slider r0Slider;
  @FXML
  private Button btnPlayMovie;

  @FXML
  private void closeApplication(ActionEvent event)
  {
    System.exit(0);
  }
    
  @FXML
  private void startPage(ActionEvent event)
  {
    try
    {
      Parent homepageLayout;
      homepageLayout = FXMLLoader.load(getClass().getResource("/edu/pitt/phdl/frednavigator/homepage.fxml"));
      Scene hompageScene = new Scene(homepageLayout);
      FredNavigatorContext.getInstance().getFredNavigatorStage().setScene(hompageScene);     
    }
    catch (IOException ex)
    {
      Logger.getLogger(HomepageController.class.getName()).log(Level.SEVERE, null, ex);
    }
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

  @FXML
  private void openMovie()
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
  
  private void makeLineChartOnSelectedTab(String r0Value, Tab selectedTab)
  {
    //If we are on the SEIR tab
    if(selectedTab == this.tabSEIR)
    {
      NumberAxis xAxis = new NumberAxis();
      NumberAxis yAxis = new NumberAxis(0.0, 1200000.0, 100000.0);
      xAxis.setLabel("Days");
      yAxis.setLabel("Number of People");


      //creating the chart
      LineChart<Number, Number> lineChart = new LineChart<>(xAxis, yAxis);
      lineChart.setTitle("SEIR: R0=" + r0Value);
      
      
      XYChart.Series seriesS = makeLineChartSeries("S", r0Value, "Susceptible");
      lineChart.getData().add(seriesS);
      XYChart.Series seriesE = makeLineChartSeries("E", r0Value, "Exposed");
      lineChart.getData().add(seriesE);
      XYChart.Series seriesI = makeLineChartSeries("I", r0Value, "Infected");
      lineChart.getData().add(seriesI);
       XYChart.Series seriesR = makeLineChartSeries("R", r0Value, "Recovered");
      lineChart.getData().add(seriesR);
      lineChart.setCreateSymbols(false);
      this.brdrPaneSEIR.setCenter(lineChart);
      
    } 
    else if(selectedTab == this.tabAttackRate) 
    {
      NumberAxis xAxis = new NumberAxis();
      NumberAxis yAxis = new NumberAxis(0.0, 100.0, 5.0);
      xAxis.setLabel("Days");
      yAxis.setLabel("% Infected");

      //creating the chart
      LineChart<Number, Number> lineChart = new LineChart<>(xAxis, yAxis);
      lineChart.setTitle("Attack Rate: R0=" + r0Value);

      XYChart.Series seriesS = makeLineChartSeries("AR", r0Value, "Attack Rate");
      lineChart.getData().add(seriesS);
      XYChart.Series seriesE = makeLineChartSeries("ARs", r0Value, "Clinical Attack Rate");
      lineChart.getData().add(seriesE);
      lineChart.setCreateSymbols(false);
      this.brdrPaneAttackRate.setCenter(lineChart);
    
    } 
    else if(selectedTab == this.tabPrevalence) 
    {
      NumberAxis xAxis = new NumberAxis();
      NumberAxis yAxis = new NumberAxis(0.0, 550000.0, 50000.0);
      xAxis.setLabel("Days");
      yAxis.setLabel("Number of People");

      //creating the chart
      LineChart<Number, Number> lineChart = new LineChart<>(xAxis, yAxis);
      lineChart.setTitle("Prevalence: R0=" + r0Value);

      XYChart.Series seriesS = makeLineChartSeries("P", r0Value, "Prevalence");
      lineChart.getData().add(seriesS);
//      XYChart.Series seriesE = makeLineChartSeries("ARs", r0Value, "Clinical Attack Rate");
//      lineChart.getData().add(seriesE);
      lineChart.setCreateSymbols(false);
      this.brdrPanePrevalence.setCenter(lineChart);
    } 
    else if(selectedTab == this.tabIncidence) 
    {
      NumberAxis xAxis = new NumberAxis();
      NumberAxis yAxis = new NumberAxis(0.0, 120000.0, 10000.0);
      xAxis.setLabel("Days");
      yAxis.setLabel("Number of People");

      //creating the chart
      LineChart<Number, Number> lineChart = new LineChart<>(xAxis, yAxis);
      lineChart.setTitle("Incidence: R0=" + r0Value);

      XYChart.Series seriesS = makeLineChartSeries("C", r0Value, "Incidence");
      lineChart.getData().add(seriesS);
      XYChart.Series seriesE = makeLineChartSeries("Cs", r0Value, "Symptomatic Incidence");
      lineChart.getData().add(seriesE);
      lineChart.setCreateSymbols(false);
      this.brdrPaneIncidence.setCenter(lineChart);    
    } 

  }

  private XYChart.Series makeLineChartSeries(String variable, String r0Value, String seriesName)
  {
    //defining a series
    XYChart.Series series = new XYChart.Series();
    series.setName(seriesName);

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

    return series;
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
        if(new_val_string.charAt(3) == '0')
        {
          r0Value = new_val_string.charAt(0) + "."
                  + new_val_string.charAt(1);
//          if (r0Value.charAt(0) > '1' && r0Value.charAt(2) == '0')
//          {
//            r0Value = r0Value.charAt(0) + "";
//          }
          
          ObservableList<Tab> tabs = tabPaneTopic1.getTabs();
          Tab selectedTab = null;
          for(Tab tab : tabs) {
            if(tab.isSelected()) 
            {
              selectedTab = tab;
              break;
            }
          }
          makeLineChartOnSelectedTab(r0Value, selectedTab);
        }
      }
    });
    
    //Add Change Listener to Tab Pane so that when the user switches tabs, it will have the correct graph
    // already populated
    this.tabPaneTopic1.getSelectionModel().selectedItemProperty().addListener(new ChangeListener<Tab>() {
      @Override 
      public void changed(ObservableValue<? extends Tab> tab, Tab oldTab, Tab newTab) {
        makeLineChartOnSelectedTab(r0Value, newTab);
      }
    });
    
    //Have the correct graph populated when the page is initialized (should be SEIR Tab)
    makeLineChartOnSelectedTab(this.r0Value, this.tabSEIR);
  }
}
