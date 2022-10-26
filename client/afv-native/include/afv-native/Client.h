/* Client.h
 *
 * This file is part of AFV-Native.
 *
 * Copyright (c) 2019 Christopher Collins
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef AFV_NATIVE_CLIENT_H
#define AFV_NATIVE_CLIENT_H

#include "afv-native/afv/RadioSimulation.h"

#include <memory>
#include <event2/event.h>

#include "afv-native/event.h"
#include "afv-native/afv/APISession.h"
#include "afv-native/afv/EffectResources.h"
#include "afv-native/afv/VoiceSession.h"
#include "afv-native/afv/dto/Transceiver.h"
#include "afv-native/audio/AudioDevice.h"
#include "afv-native/event/EventCallbackTimer.h"
#include "afv-native/http/EventTransferManager.h"
#include "afv-native/http/RESTRequest.h"

namespace afv_native {
    /** Client provides a fully functional PilotClient that can be integrated into
     * an application.
     */
    class Client {
    public:
        /** Construct an AFV-native Pilot Client.
         *
         * The pilot client will be in the disconnected state and ready to have
         * the credentials, position and other configuration options set before
         * attempting to connect.
         *
         * The containing client must provide and run a libevent eventloop for
         * AFV-native to attach its operations against, and must ensure that
         * this loop is run constantly, even when the client is not connected.
         * (It's used for some tear-down operations which must run to completion
         * after the client is shut-down if possible.)
         *
         * @param evBase an initialised libevent event_base to register the client's
         *      asynchronous IO and deferred operations against.
         * @param resourceBasePath A relative or absolute path to where the AFV-native
         *      resource files are located.
         * @param baseUrl The baseurl for the AFV API server to connect to.  The
         *      default should be used in most cases.
         * @param numRadios The number of transceivers to instantiate for this
         *      client.
         * @param clientName The name of this client to advertise to the
         *      audio-subsystem.
         */
        Client(
                struct event_base *evBase,
                unsigned int numRadios = 2,
                const std::string &clientName = "AFV-Native",
                std::string baseUrl = "https://voice1.vatsim.uk");

        virtual ~Client();

        /** setBaseUrl is used to change the API URL.
         *
         * @note This affects all future API requests, but any in flight will not
         * be cancelled and resent.
         *
         * @param newUrl the new URL (without at trailing slash).
         */
        void setBaseUrl(std::string newUrl);

        /** set the ClientPosition to report with the next transceiver update.
         *
         * @param lat the client latitude in decimal degrees.
         * @param lon the client longitude in decimal degrees.
         * @param amslm the client's altitude above mean-sea-level in meters.
         * @param aglm the client's altitude above ground level in meters.
         */
        void setClientPosition(double lat, double lon, double amslm, double aglm);

        /** set the radio frequency for the nominated radio.
         *
         * This method will invoke an immediate transceiver set update.
         *
         * @param radioNum the ordinal of the radio to tune
         * @param freq the frequency in Hz
         */
        void setRadioState(unsigned int radioNum, int freq);

        /** set the radio the Ptt will control.
         *
         * @param radioNum the ordinal fo the radio that will transmit when PTT
         *     is pressed.
         */
        void setTxRadio(unsigned int radioNum);

        /** sets the (linear) gain to be applied to radioNum */
        void setRadioGain(unsigned int radioNum, float gain);

        void setMicrophoneVolume(float volume);

        /** sets the PTT (push-to-talk) state for the radio.
         *
         * @note If the radio frequencies are out of sync with the server, this will
         * initiate an immediate transceiver set update and the Ptt will remain
         * blocked/guarded until the update has been acknowledged.
         *
         * @param pttState true to start transmitting, false otherwise.
         */
        void setPtt(bool pttState);

        /** setCredentials sets the user Credentials for this client.
         *
         * @note This only affects future attempts to connect.
         *
         * @param username The user's CID or username.
         * @param password The user's password
         */
        void setCredentials(const std::string &username, const std::string &password);

        /** setCallsign sets the user's callsign for this client.
         *
         * @note This only affects future attempts to connect.
         *
         * @param callsign the callsign to use.
         */
        void setCallsign(std::string callsign);

        /** set the audioApi (per the audio::AudioDevice definitions) to use when next starting the
         * audio system.
         * @param api an API id
         */
        void setAudioApi(audio::AudioDevice::Api api);

        void setAudioInputDevice(std::string inputDevice);
        void setAudioOutputDevice(std::string outputDevice);

        /** isAPIConnected() indicates if the API Server connection is up.
         *
         * @return true if the API server connection is good or in the middle of
         * reauthenticating a live session.  False if it is yet to authenticate,
         * or is not connected at all.
         */
        bool isAPIConnected() const;
        bool isVoiceConnected() const;

        /** connect() starts the Client connecting to the API server, then establishing the voice session.
         *
         * In order for this to work, the credentials, callsign and audio device must be configured first,
         * and you should have already set the clientPosition and radio states once.
         *
         * @return true if the connection process was able to start, false if key data is missing or an
         *  early failure occured.
         */
        bool connect();

        /** disconnect() tears down the voice session and discards the authentication session data.
         *
         */
        void disconnect();

        double getInputPeak() const;
        double getInputVu() const;

        bool getEnableInputFilters() const;
        void setEnableInputFilters(bool enableInputFilters);
        void setEnableOutputEffects(bool enableEffects);
        void setEnableHfSquelch(bool enableSquelch);

        /** ClientEventCallback provides notifications when certain client events occur.  These can be used to
         * provide feedback within the client itself without needing to poll Client's methods.
         *
         * The callbacks take two paremeters-  the first is the ClientEventType which informs the client what type
         * of event occured.
         *
         * The second argument is a pointer to data relevant to the callback.  The memory it points to is only
         * guaranteed to be available for the duration of the callback.
         */
        util::ChainedCallback<void(ClientEventType,void*)>  ClientEventCallback;

        /** getStationAliases returns a vector of all the known station aliases.
         *
         * @note this method uses a copy in place to prevent race inside the
         *  client code and consumers and is consequentially expensive.  Please
         *  only call it after you get a notification of there being a change,
         *  and even then, only once.
         */
        std::vector<afv::dto::Station> getStationAliases() const;

        void startAudio();
        void stopAudio();

        /** logAudioStatistics dumps the internal data about over/underflow totals to the AFV log.
         *
         */
        void logAudioStatistics();

        std::shared_ptr<const afv::RadioSimulation> getRadioSimulation() const;
        std::shared_ptr<const audio::AudioDevice> getAudioDevice() const;

        /** getRxActive returns if the nominated radio is currently Receiving voice, irrespective as to if it's audiable
         * or not.
         *
         * @param radioNumber the number (starting from 0) of the radio to probe
         * @return true if the radio would have voice to play, false otherwise.
         */
        bool getRxActive(unsigned int radioNumber);

        /** getTxActive returns if the nominated radio is currently Transmitting voice.
         *
         * @param radioNumber the number (starting from 0) of the radio to probe
         * @return true if the radio is transmitting from the mic, false otherwise.
         */
        bool getTxActive(unsigned int radioNumber);

    protected:
        struct ClientRadioState {
            int mCurrentFreq;
            int mNextFreq;
        };

        struct event_base *mEvBase;
        std::shared_ptr<afv::EffectResources> mFxRes;

        http::EventTransferManager mTransferManager;
        afv::APISession mAPISession;
        afv::VoiceSession mVoiceSession;
        std::shared_ptr<afv::RadioSimulation> mRadioSim;
        std::shared_ptr<audio::AudioDevice> mAudioDevice;

        double mClientLatitude;
        double mClientLongitude;
        double mClientAltitudeMSLM;
        double mClientAltitudeGLM;
        std::vector<struct ClientRadioState> mRadioState;

        std::string mCallsign;

        void sessionStateCallback(afv::APISessionState state);
        void voiceStateCallback(afv::VoiceSessionState state);

        bool mTxUpdatePending;
        bool mWantPtt;
        bool mPtt;

        bool areTransceiversSynced() const;
        std::vector<afv::dto::Transceiver> makeTransceiverDto();
        /* sendTransceiverUpdate sends the update now, in process.
         * queueTransceiverUpdate schedules it for the next eventloop.  This is a
         * critical difference as it prevents bad reentrance as the transceiver
         * update callback can trigger a second update if the desired state is
         * out of sync! */
        void sendTransceiverUpdate();
        void queueTransceiverUpdate();
        void stopTransceiverUpdate();

        void aliasUpdateCallback();
    private:
        void unguardPtt();
    protected:
        event::EventCallbackTimer mTransceiverUpdateTimer;

        std::string mClientName;
        audio::AudioDevice::Api mAudioApi;
        std::string mAudioInputDeviceName;
        std::string mAudioOutputDeviceName;
    public:
    };
}

#endif //AFV_NATIVE_CLIENT_H
