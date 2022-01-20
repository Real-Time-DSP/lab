% This script will demodulate a PAM recording and show the bits as an image
% Click Run at the top or type 'QAM_receiver_demo' in the command window
classdef QAM_receiver_demo < matlab.apps.AppBase

    % Properties that correspond to app components
    properties (Access = public)
        UIFigure                    matlab.ui.Figure
        GridLayout                  matlab.ui.container.GridLayout
        LeftPanel                   matlab.ui.container.Panel
        RunButton                   matlab.ui.control.Button
        SymbolfreqEditField         matlab.ui.control.NumericEditField
        SymbolfreqEditFieldLabel    matlab.ui.control.Label
        SymbolphaseEditField        matlab.ui.control.NumericEditField
        SymbolphaseEditFieldLabel   matlab.ui.control.Label
        CarrierfreqEditField        matlab.ui.control.NumericEditField
        CarrierfreqEditFieldLabel   matlab.ui.control.Label
        CarrierphaseEditField       matlab.ui.control.NumericEditField
        CarrierphaseEditFieldLabel  matlab.ui.control.Label
        VariableEditField           matlab.ui.control.EditField
        VariableEditFieldLabel      matlab.ui.control.Label
        RightPanel                  matlab.ui.container.Panel
        UIAxes                      matlab.ui.control.UIAxes
    end

    % Properties that correspond to apps with auto-reflow
    properties (Access = private)
        onePanelWidth = 576;
    end

    % Callbacks that handle component events
    methods (Access = private)

        % Button pushed function: RunButton
        function run_receiver(app, event)
            v = app.VariableEditField.Value; r = load([v ,'.mat']);
            f = fieldnames(r); r = getfield(r,f{1});
            p_carrier = app.CarrierphaseEditField.Value;
            f_carrier = app.CarrierfreqEditField.Value;
            p_symbol = app.SymbolphaseEditField.Value;
            f_symbol = app.SymbolfreqEditField.Value;
            fs = 48000; sps = 16; n = (1:length(r)) - 1;
            w0 = 2*pi*f_carrier/fs; N_symbol = round(fs/f_symbol);
            carrier_offset = round(p_carrier/w0);
            symbol_offset = round(N_symbol*p_symbol/pi);
            pulse_shaping_filter = rcosdesign(0.8,4,sps,'normal');
            group_delay = (length(pulse_shaping_filter)-1)/2;
            
            demodulated_c = r .* cos(w0*(n + carrier_offset));
            demodulated_s = r .* sin(w0*(n + carrier_offset));
            recovered_pulses_c = conv(demodulated_c,pulse_shaping_filter);
            recovered_pulses_s = conv(demodulated_s,pulse_shaping_filter);
            recovered_pulses_c(1:(group_delay + symbol_offset)) = [];
            recovered_pulses_s(1:(group_delay + symbol_offset)) = [];
            
            recovered_data_c = downsample(recovered_pulses_c,N_symbol) > 0;
            recovered_data_s = downsample(recovered_pulses_s,N_symbol) > 0;
            recovered_data = [recovered_data_c;recovered_data_s];
            recovered_data = recovered_data(:);
            ncol = floor(length(recovered_data)/128);
            img = recovered_data(1:(128*ncol));
            imshow(reshape(img,[128,ncol]),'parent',app.UIAxes);
            
        end

        % Changes arrangement of the app based on UIFigure width
        function updateAppLayout(app, event)
            currentFigureWidth = app.UIFigure.Position(3);
            if(currentFigureWidth <= app.onePanelWidth)
                % Change to a 2x1 grid
                app.GridLayout.RowHeight = {486, 486};
                app.GridLayout.ColumnWidth = {'1x'};
                app.RightPanel.Layout.Row = 2;
                app.RightPanel.Layout.Column = 1;
            else
                % Change to a 1x2 grid
                app.GridLayout.RowHeight = {'1x'};
                app.GridLayout.ColumnWidth = {220, '1x'};
                app.RightPanel.Layout.Row = 1;
                app.RightPanel.Layout.Column = 2;
            end
        end
    end

    % Component initialization
    methods (Access = private)

        % Create UIFigure and components
        function createComponents(app)

            % Create UIFigure and hide until all components are created
            app.UIFigure = uifigure('Visible', 'off');
            app.UIFigure.AutoResizeChildren = 'off';
            app.UIFigure.Position = [100 100 1087 486];
            app.UIFigure.Name = 'MATLAB App';
            app.UIFigure.SizeChangedFcn = createCallbackFcn(app, @updateAppLayout, true);

            % Create GridLayout
            app.GridLayout = uigridlayout(app.UIFigure);
            app.GridLayout.ColumnWidth = {220, '1x'};
            app.GridLayout.RowHeight = {'1x'};
            app.GridLayout.ColumnSpacing = 0;
            app.GridLayout.RowSpacing = 0;
            app.GridLayout.Padding = [0 0 0 0];
            app.GridLayout.Scrollable = 'on';

            % Create LeftPanel
            app.LeftPanel = uipanel(app.GridLayout);
            app.LeftPanel.Layout.Row = 1;
            app.LeftPanel.Layout.Column = 1;

            % Create VariableEditFieldLabel
            app.VariableEditFieldLabel = uilabel(app.LeftPanel);
            app.VariableEditFieldLabel.HorizontalAlignment = 'right';
            app.VariableEditFieldLabel.Position = [31 368 49 22];
            app.VariableEditFieldLabel.Text = 'Variable';

            % Create VariableEditField
            app.VariableEditField = uieditfield(app.LeftPanel, 'text');
            app.VariableEditField.Position = [95 368 100 22];
            app.VariableEditField.Value = 'QAM';

            % Create CarrierphaseEditFieldLabel
            app.CarrierphaseEditFieldLabel = uilabel(app.LeftPanel);
            app.CarrierphaseEditFieldLabel.HorizontalAlignment = 'right';
            app.CarrierphaseEditFieldLabel.Position = [41 276 42 28];
            app.CarrierphaseEditFieldLabel.Text = {'Carrier'; 'phase'};

            % Create CarrierphaseEditField
            app.CarrierphaseEditField = uieditfield(app.LeftPanel, 'numeric');
            app.CarrierphaseEditField.ValueDisplayFormat = '%11.4f';
            app.CarrierphaseEditField.Position = [98 282 100 22];

            % Create CarrierfreqEditFieldLabel
            app.CarrierfreqEditFieldLabel = uilabel(app.LeftPanel);
            app.CarrierfreqEditFieldLabel.HorizontalAlignment = 'right';
            app.CarrierfreqEditFieldLabel.Position = [41 229 42 28];
            app.CarrierfreqEditFieldLabel.Text = {'Carrier'; 'freq.'};

            % Create CarrierfreqEditField
            app.CarrierfreqEditField = uieditfield(app.LeftPanel, 'numeric');
            app.CarrierfreqEditField.ValueDisplayFormat = '%11.4f';
            app.CarrierfreqEditField.Position = [98 235 100 22];
            app.CarrierfreqEditField.Value = 12000;

            % Create SymbolphaseEditFieldLabel
            app.SymbolphaseEditFieldLabel = uilabel(app.LeftPanel);
            app.SymbolphaseEditFieldLabel.HorizontalAlignment = 'right';
            app.SymbolphaseEditFieldLabel.Position = [37 180 46 28];
            app.SymbolphaseEditFieldLabel.Text = {'Symbol'; 'phase'};

            % Create SymbolphaseEditField
            app.SymbolphaseEditField = uieditfield(app.LeftPanel, 'numeric');
            app.SymbolphaseEditField.ValueDisplayFormat = '%11.4f';
            app.SymbolphaseEditField.Position = [98 186 100 22];

            % Create SymbolfreqEditFieldLabel
            app.SymbolfreqEditFieldLabel = uilabel(app.LeftPanel);
            app.SymbolfreqEditFieldLabel.HorizontalAlignment = 'right';
            app.SymbolfreqEditFieldLabel.Position = [37 133 46 28];
            app.SymbolfreqEditFieldLabel.Text = {'Symbol'; 'freq.'};

            % Create SymbolfreqEditField
            app.SymbolfreqEditField = uieditfield(app.LeftPanel, 'numeric');
            app.SymbolfreqEditField.ValueDisplayFormat = '%11.4f';
            app.SymbolfreqEditField.Position = [98 139 100 22];
            app.SymbolfreqEditField.Value = 3000;

            % Create RunButton
            app.RunButton = uibutton(app.LeftPanel, 'push');
            app.RunButton.ButtonPushedFcn = createCallbackFcn(app, @run_receiver, true);
            app.RunButton.Position = [68 52 100 22];
            app.RunButton.Text = 'Run';

            % Create RightPanel
            app.RightPanel = uipanel(app.GridLayout);
            app.RightPanel.Layout.Row = 1;
            app.RightPanel.Layout.Column = 2;

            % Create UIAxes
            app.UIAxes = uiaxes(app.RightPanel);
            app.UIAxes.XTick = [];
            app.UIAxes.YTick = [];
            app.UIAxes.Position = [10 6 832 463];

            % Show the figure after all components are created
            app.UIFigure.Visible = 'on';
        end
    end

    % App creation and deletion
    methods (Access = public)

        % Construct app
        function app = QAM_receiver_demo

            % Create UIFigure and components
            createComponents(app)

            % Register the app with App Designer
            registerApp(app, app.UIFigure)

            if nargout == 0
                clear app
            end
        end

        % Code that executes before app deletion
        function delete(app)

            % Delete UIFigure when app is deleted
            delete(app.UIFigure)
        end
    end
end