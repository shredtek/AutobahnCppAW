///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) Tavendo GmbH
//
// Boost Software License - Version 1.0 - August 17th, 2003
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef AUTOBAHN_WAMP_TRANSPORT_HPP
#define AUTOBAHN_WAMP_TRANSPORT_HPP

#include "boost_config.hpp"

#include <boost/thread/future.hpp>
#include <memory>
#include <string>

namespace autobahn {

class wamp_message;
class wamp_transport_handler;

/*!
 * Provides an abstraction for a transport to be used by the session. A wamp
 * transport is defined as being message based, bidirectional, reliable, and
 * ordered.
 */
class wamp_transport
{
public:
    /*!
     * Handler to invoke when pausing transport transmission.
     */
    using pause_handler = std::function<void()>;

    /*!
     * Handler to invoke when resuming transport transmission.
     */
    using resume_handler = std::function<void()>;

public:
    /*!
     * Default virtual destructor.
     */
    virtual ~wamp_transport() = default;

    /*
     * CONNECTION INTERFACE
     */
    /*!
     * Attempts to connect the transport.
     *
     * @return A future that will be satisfied when the connect attempt
     *         has been made.
     */
    virtual boost::future<void> connect() = 0;

    /*!
     * Attempts to disconnect the transport.
     *
     * @return A future that will be satisfied when the disconnect attempt
     *         has been made.
     */
    virtual boost::future<void> disconnect() = 0;

    /*!
     * Determines if the transport is connected.
     *
     * @return Whether or not the transport is connected.
     */
    virtual bool is_connected() const = 0;

    /*
     * SENDER INTERFACE
     */
    /*!
     * Send the message synchronously over the transport.
     *
     * @param message The message to be sent.
     */
    virtual void send_message(wamp_message&& message) = 0;

    /*!
     * Set the handler to be invoked when the transport detects congestion
     * sending to the remote peer and needs to apply backpressure on the
     * application.
     *
     * @param handler The pause handler to be invoked.
     */
    virtual void set_pause_handler(pause_handler&& handler) = 0;

    /*!
     * Set the handler to be invoked when the transport detects congestion
     * has subsided on the remote peer and the application can resume sending
     * messages.
     *
     * @param handler The resume handler to be invoked.
     */
    virtual void set_resume_handler(resume_handler&& handler) = 0;

    /*
     * RECEIVER INTERFACE
     */
    /*!
     * Pause receiving of messages. This will prevent the transport from receiving
     * any more messages until it has been resumed. This is used to excert
     * backpressure on the sending peer.
     */
    virtual void pause() = 0;

    /*!
     * Resume receiving of messages. The transport will now begin receiving messsages
     * again and lift backpressure from the sending peer.
     */
    virtual void resume() = 0;

    /*!
     * Attaches a handler to the transport. Only one handler may
     * be attached at any given time.
     *
     * @param handler The handler to attach to this transport.
     */
    virtual void attach(
            const std::shared_ptr<wamp_transport_handler>& handler) = 0;

    /*!
     * Detaches the handler currently attached to the transport.
     */
    virtual void detach() = 0;

    /*!
     * Determines if the transport has a handler attached.
     *
     * @return Whether or not a handler is attached.
     */
    virtual bool has_handler() const = 0;
};

} // namespace autobahn

#endif // AUTOBAHN_WAMP_TRANSPORT_HPP
